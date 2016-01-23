package tests.registry;

import java.io.IOException;
import java.net.BindException;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Arrays;
import java.util.HashSet;
import java.util.LinkedHashSet;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;
import java.util.TreeMap;
import java.util.TreeSet;

public class Registry implements Runnable {

   private /* */ InetSocketAddress      _thisEndPoint;
   private final Set<InetSocketAddress> _endPoints  = new TreeSet<>( new InetSocketAddressComparator());
   private final Map<InetSocketAddress,
      Receiver>                         _receivers  = new TreeMap<>( new InetSocketAddressComparator());
   private final Set<Sender>            _senders    = new LinkedHashSet<>();
   private final Map<InetSocketAddress,
      TreeSet<Topic>>                   _topics     = new TreeMap<>( new InetSocketAddressComparator());
   private final Map<InetSocketAddress,
      TreeSet<String>>                  _interfaces = new TreeMap<>( new InetSocketAddressComparator());
   private final Set<
      IRegistryListener>                _listeners  = new HashSet<>();
   private final int                    _bootPort;
   private final MessageDigest          _md5Builder = MessageDigest.getInstance( "md5" );
   private final ServerSocketChannel    _server     = ServerSocketChannel.open();

   public Registry( String host, final int bootPort ) throws IOException, NoSuchAlgorithmException {
      _bootPort = bootPort;
      for( int port = _bootPort; true; ++port ) {
         try {
            _server.bind( new InetSocketAddress( host, port ));
            System.err.printf( "%s|server listening on port %d\n",
               getClass().getName(), port );
            synchronized( _endPoints ) {
               _thisEndPoint = (InetSocketAddress)_server.getLocalAddress();
               _endPoints.add( _thisEndPoint );
            }
            new Thread( this ).start();
            if( port != _bootPort ) {
               createReceiver( new InetSocketAddress( host, _bootPort ));
            }
            break;
         }
         catch( final BindException x ) {/**/}
      }
   }

   private static void write( ByteBuffer payload, SocketChannel channel ) throws IOException {
      final ByteBuffer header = ByteBuffer.allocate( 4 );
      header.putInt( payload.remaining());
      header.flip();
      while( header.hasRemaining()) {
         channel.write( header );
      }
      while( payload.hasRemaining()) {
         channel.write( payload );
      }
   }

   private static ByteBuffer read( SocketChannel channel ) throws IOException {
      final ByteBuffer header   = ByteBuffer.allocate( 4 );
      while( header.position() < 4 ) {
         if( channel.read( header ) < 0 ) {
            return null;
         }
      }
      header.flip();
      final int length = header.getInt();
      final ByteBuffer payload  = ByteBuffer.allocate( 64*1024 );
      while( payload.position() < length ) {
         if( channel.read( payload ) < 0 ) {
            return null;
         }
      }
      payload.flip();
      return payload;
   }

   private static void serializeEndPoint( InetSocketAddress endPoint, ByteBuffer to ) {
      final String host   = endPoint.getHostString();
      final int    port   = endPoint.getPort();
      final byte[] hostB  = host.getBytes();
      to.putInt( hostB.length );
      to.put   ( hostB );
      to.putInt( port );
   }

   private static InetSocketAddress unserializeEndPoint( ByteBuffer from ) {
      final int    length = from.getInt();
      final byte[] hostB  = new byte[length];
      from.get( hostB );
      final String host   = new String( hostB );
      final int    port   = from.getInt();
      return new InetSocketAddress( host, port );
   }

   @Override
   public void run() {
      System.err.printf( "%s|server thread running\n", getClass().getName());
      for( int err = 0; err < 3; ++err ) {
         try {
            @SuppressWarnings("resource")
            final SocketChannel channel = _server.accept();
            final Sender sender = new Sender( channel, Registry.this );
            synchronized( _senders ) {
               _senders.add( sender );
            }
            final ByteBuffer payload = read( channel );
            if( payload == null ) {
               break;
            }
            createReceiver( unserializeEndPoint( payload ));
            sender.send();
            err = 0;
         }
         catch( final IOException e ) {
            e.printStackTrace();
         }
      }
      System.err.printf( "%s|server thread ended\n", getClass().getName());
   }

   private void createReceiver( InetSocketAddress endPoint ) throws IOException {
      synchronized( _receivers ) {
         if( _receivers.containsKey( endPoint )) {
            return;
         }
      }
      final String host = endPoint.getHostString();
      @SuppressWarnings("resource")
      final SocketChannel channel = SocketChannel.open();
      for( int port = _bootPort + 1; true; ++port ) {
         try {
            channel.bind( new InetSocketAddress( host, port ));
            channel.connect( endPoint );
            synchronized( _receivers ) {
               _receivers.put( endPoint, new Receiver( channel, this ));
            }
            final ByteBuffer payload = ByteBuffer.allocate( 1024 );
            serializeEndPoint( _thisEndPoint, payload );
            payload.flip();
            write( payload, channel );
            break;
         }
         catch( final BindException x ) {/**/}
      }
   }

   private void computeMd5() {
      _md5Builder.reset();
      synchronized( _endPoints ) {
         for( final InetSocketAddress addr : _endPoints ) {
            _md5Builder.update( addr.toString().getBytes());
         }
      }
      synchronized( _topics ) {
         for( final Entry<InetSocketAddress, TreeSet<Topic>> e : _topics.entrySet()) {
            final InetSocketAddress addr   = e.getKey();
            final TreeSet<Topic>    topics = e.getValue();
            _md5Builder.update( addr.toString().getBytes());
            for( final Topic topic : topics ) {
               _md5Builder.update( String.format( "%d", topic._value ).getBytes());
            }
         }
      }
      synchronized( _interfaces ) {
         for( final Entry<InetSocketAddress, TreeSet<String>> e : _interfaces.entrySet()) {
            final InetSocketAddress addr    = e.getKey();
            final TreeSet<String>   intrfcs = e.getValue();
            _md5Builder.update( addr.toString().getBytes());
            for( final String intrfc : intrfcs ) {
               _md5Builder.update( intrfc.getBytes());
            }
         }
      }
   }

   void writeTo( SocketChannel channel ) throws IOException {
      final ByteBuffer payload = ByteBuffer.allocate( 64*1024 );
      computeMd5();
      final byte[] digest = _md5Builder.digest();
      payload.put((byte)digest.length );
      payload.put( digest );
      synchronized( _endPoints ) {
         payload.putInt( _endPoints.size());
         for( final InetSocketAddress endPoint : _endPoints ) {
            serializeEndPoint( endPoint, payload );
         }
      }
      synchronized( _topics ) {
         payload.putInt( _topics.size());
         for( final Entry<InetSocketAddress, TreeSet<Topic>> e : _topics.entrySet()) {
            serializeEndPoint( e.getKey(), payload );
            final Set<Topic> topics = e.getValue();
            payload.putInt( topics.size());
            for( final Topic topic : topics ) {
               topic.serialize( payload );
            }
         }
      }
      synchronized( _interfaces ) {
         payload.putInt( _interfaces.size());
         for( final Entry<InetSocketAddress, TreeSet<String>> e : _interfaces.entrySet()) {
            serializeEndPoint( e.getKey(), payload );
            final Set<String> intrfcs = e.getValue();
            payload.putInt( intrfcs.size());
            for( final String intrfc : intrfcs ) {
               final byte[] intrfcB = intrfc.getBytes();
               payload.putInt( intrfcB.length );
               payload.put   ( intrfcB );
            }
         }
      }
      payload.flip();
      write( payload, channel );
   }

   private void send() {
      synchronized( _senders ) {
         System.err.println( getClass().getName() + ".send|senders.size() = " + _senders.size());
         for( final Sender sender : _senders ) {
            System.err.println( getClass().getName() + ".send|ask sender to send registry" );
            sender.send();
         }
      }
   }

   private void fireRegistryHasChanged() {
      synchronized( _listeners ) {
         for( final IRegistryListener l : _listeners ) {
            l.registryHasChanged( this );
         }
      }
   }

   boolean merge( SocketChannel from ) throws IOException {
      final ByteBuffer payload = read( from );
      if( payload == null ) {
         return false;
      }
      computeMd5(); // MD5 before merge
      boolean      modified    = false;
      final int    sumSize     = payload.get();
      final byte[] receivedSum = new byte[sumSize];
      payload.get( receivedSum );
      synchronized( _endPoints ) {
         for( int i = 0, count = payload.getInt(); i < count; ++i ) {
            final InetSocketAddress addr   = unserializeEndPoint( payload );
            if( _endPoints.add( addr )) {
               modified = true;
            }
         }
      }
      synchronized( _topics ) {
         for( int i = 0, count = payload.getInt(); i < count; ++i ) {
            final int length = payload.getInt();
            final byte[]            hostB  = new byte[length];
            payload.get( hostB );
            final String            host   = new String( hostB );
            final int               port   = payload.getInt();
            final InetSocketAddress addr   = new InetSocketAddress( host, port );
            TreeSet<Topic> topics = _topics.get( addr );
            if( topics == null ) {
               _topics.put( addr, topics = new TreeSet<>());
               modified = true;
            }
            for( int j = 0, topicsCount = payload.getInt(); j < topicsCount; ++j ) {
               if( topics.add( Topic.unserialize( payload ))) {
                  modified = true;
               }
            }
         }
      }
      synchronized( _interfaces ) {
         for( int i = 0, count = payload.getInt(); i < count; ++i ) {
            final int length = payload.getInt();
            final byte[]            hostB   = new byte[length];
            payload.get( hostB );
            final String            host    = new String( hostB );
            final int               port    = payload.getInt();
            final InetSocketAddress addr    = new InetSocketAddress( host, port );
            TreeSet<String> intrfcs = _interfaces.get( addr );
            if( intrfcs == null ) {
               _interfaces.put( addr, intrfcs = new TreeSet<>());
               modified = true;
            }
            for( int j = 0, interfaceCount = payload.getInt(); j < interfaceCount; ++j ) {
               final int    len     = payload.getInt();
               final byte[] intrfcB = new byte[len];
               payload.get( intrfcB );
               final String intrfc  = new String( intrfcB );
               if( intrfcs.add( intrfc )) {
                  modified = true;
               }
            }
         }
      }
      synchronized( _endPoints ) {
         for( final InetSocketAddress endPoint : _endPoints ) {
            if( ! _thisEndPoint.equals( endPoint )) {
               createReceiver( endPoint );
            }
         }
      }
      final boolean sameSum = Arrays.equals( receivedSum, _md5Builder.digest());
      System.err.println( "sameSum: " + sameSum );
      if( modified && ! sameSum ) {
         send();
      }
      fireRegistryHasChanged();
      return true;
   }

   public InetSocketAddress getThisEndPoint() {
      return _thisEndPoint;
   }

   public Set<InetSocketAddress> getEndPoints() {
      return _endPoints;
   }

   public void addListener( IRegistryListener listener ) {
      _listeners.add( listener );
   }

   public void addTopic( Topic topic ) {
      TreeSet<Topic> topics = _topics.get( _thisEndPoint );
      if( topics == null ) {
         _topics.put( _thisEndPoint, topics = new TreeSet<>());
      }
      topics.add( topic );
      send();
      fireRegistryHasChanged();
   }

   public Map<InetSocketAddress, TreeSet<Topic>> getTopics() {
      return _topics;
   }

   public void addInterface( String intrfc ) {
      TreeSet<String> intrfcs = _interfaces.get( _thisEndPoint );
      if( intrfcs == null ) {
         _interfaces.put( _thisEndPoint, intrfcs = new TreeSet<>());
      }
      intrfcs.add( intrfc );
      send();
      fireRegistryHasChanged();
   }

   public Map<InetSocketAddress, TreeSet<String>> getInterfaces() {
      return _interfaces;
   }

   public void close() throws IOException {
      for( final Receiver receiver : _receivers.values()) {
         receiver.close();
      }
      for( final Sender sender: _senders ) {
         sender.close();
      }
   }
}
