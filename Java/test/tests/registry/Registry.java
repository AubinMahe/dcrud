package tests.registry;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;
import java.util.TreeMap;
import java.util.TreeSet;
import java.util.function.Consumer;

public class Registry {

   public static final boolean LOG = false;

   private final Map<InetSocketAddress,
      Relation>                         _relations  = new TreeMap<>( new InetSocketAddressComparator());
   private final Set<InetSocketAddress> _endPoints  = new TreeSet<>( new InetSocketAddressComparator());
   private final Map<InetSocketAddress,
      TreeSet<Topic>>                   _topics     = new TreeMap<>( new InetSocketAddressComparator());
   private final Map<InetSocketAddress,
      TreeSet<String>>                  _interfaces = new TreeMap<>( new InetSocketAddressComparator());
   private final Set<
      IRegistryListener>                _listeners  = new HashSet<>();
   private final MessageDigest          _md5Builder = MessageDigest.getInstance( "md5" );
   private final IRelationsFactory      _relationsFactory;
   private final InetSocketAddress      _thisEndPoint;

   public Registry( String host, final int bootPort, Consumer<ByteBuffer> dataConsumer ) throws IOException, NoSuchAlgorithmException {
      _relationsFactory = RelationsFactoryBuilder.createTCP( host, bootPort, this, dataConsumer );
      _thisEndPoint     = _relationsFactory.getLocalEndPoint();
   }

   void addEndPoint( InetSocketAddress endPoint ) {
      synchronized( _endPoints ) {
         _endPoints.add( endPoint );
      }
   }

   void addRelation( InetSocketAddress addr, Relation relation ) {
      synchronized( _relations ) {
         _relations.put( addr, relation );
      }
   }

   public Relation getRelation( InetSocketAddress endPoint ) {
      synchronized( _relations ) {
         return _relations.get( endPoint );
      }
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

   void serializeTo( ByteBuffer payload ) {
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
   }

   private void send() {
      synchronized( _relations ) {
         for( final Entry<InetSocketAddress, Relation> e : _relations.entrySet()) {
            if( ! e.getKey().equals( _thisEndPoint )) {
               final Relation relation = e.getValue();
               relation.sendRegistry();
            }
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

   boolean merge( ByteBuffer payload ) {
      if( LOG ) {
         System.err.printf( "%s.merge|begin\n", getClass().getName());
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
               try {
                  _relationsFactory.connectTo( endPoint );
               }
               catch( final IOException e ) {
                  e.printStackTrace();
               }
            }
         }
      }
      final boolean sameSum = Arrays.equals( receivedSum, _md5Builder.digest());
      if( modified && ! sameSum ) {
         send();
      }
      else if( LOG ) {
         System.err.printf( "%s.merge|sums are equal\n", getClass().getName());
      }
      fireRegistryHasChanged();
      if( LOG ) {
         System.err.printf( "%s.merge|end\n", getClass().getName());
      }
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
      for( final Relation relation : _relations.values()) {
         relation.close();
      }
   }
}
