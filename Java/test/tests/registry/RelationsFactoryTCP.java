package tests.registry;

import java.io.IOException;
import java.net.BindException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.NetworkInterface;
import java.nio.ByteBuffer;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;
import java.util.Enumeration;
import java.util.concurrent.CountDownLatch;
import java.util.function.Consumer;

public class RelationsFactoryTCP implements IRelationsFactory, Runnable {

   private final ServerSocketChannel  _server    = ServerSocketChannel.open();
   private final CountDownLatch       _isStarted = new CountDownLatch( 1 );
   private final int                  _bootPort;
   private final Registry             _registry;
   private final Consumer<ByteBuffer> _dataConsumer;
   private /* */ InetSocketAddress    _thisEndPoint;

   RelationsFactoryTCP(
      NetworkInterface     intrfc,
      InetAddress          registryServer,
      int                  registryPort,
      int                  bootPort,
      Registry             registry,
      Consumer<ByteBuffer> dataConsumer ) throws IOException
   {
      _bootPort     = bootPort;
      _registry     = registry;
      _dataConsumer = dataConsumer;
      final Enumeration<InetAddress> a = intrfc.getInetAddresses();
      String intrfcAddr = null;
      do {
         final InetAddress b = a.nextElement();
         if( b.getAddress().length == registryServer.getAddress().length ) {
            intrfcAddr = b.getHostAddress();
         }
      } while( intrfcAddr == null );
      boolean isPrimary = true;
      for( int port = _bootPort; true; ++port ) {
         try {
            _thisEndPoint = new InetSocketAddress( intrfcAddr, port );
            _server.bind( _thisEndPoint );
            if( Registry.LOG ) {
               System.err.printf( "%s.<ctor>|server listening on %s\n", getClass().getName(),
                  _thisEndPoint );
            }
            _registry.addEndPoint( _thisEndPoint );
            new Thread( this ).start();
            break;
         }
         catch( final BindException x ) {
            isPrimary = false;
         }
      }
      if( ! isPrimary ) {
         try {
            final InetSocketAddress isa = new InetSocketAddress( registryServer, registryPort );
            _isStarted.await();
            connectTo( isa );
         }
         catch( final IOException x ) {
            if( Registry.LOG ) {
               System.err.printf( "%s.<ctor>|%s:%d: %s\n", getClass().getName(),
                  registryServer, registryPort, x.getMessage());
            }
         }
         catch( final InterruptedException x ) {
            x.printStackTrace();
            System.exit( 2 );
         }
      }
   }

   @Override
   public InetSocketAddress getLocalEndPoint() {
      return _thisEndPoint;
   }

   private Relation connectTo( SocketChannel channel ) {
      final String   local     = channel.socket().getLocalSocketAddress ().toString();
      final String   remote    = channel.socket().getRemoteSocketAddress().toString();
      final String   channelId = String.format( "%s <==> %s", local, remote );
      final Relation relation  = new Relation( channel, channelId, _registry, _dataConsumer );
      final InetSocketAddress addr = (InetSocketAddress)channel.socket().getRemoteSocketAddress();
      _registry.addRelation( addr, relation );
      return relation;
   }

   @Override
   public void connectTo( InetSocketAddress endPoint ) throws IOException {
      final Relation relation = _registry.getRelation( endPoint );
      if( relation == null ) {
         final String host = endPoint.getHostString();
         @SuppressWarnings("resource")
         final SocketChannel channel = SocketChannel.open();
         for( int port = _bootPort + 1; true; ++port ) {
            try {
               channel.bind( new InetSocketAddress( host, port ));
               channel.connect( endPoint );
               connectTo( channel );
               break;
            }
            catch( final BindException x ) {/**/}
         }
      }
   }

   @Override
   public void run() {
      if( Registry.LOG ) {
         System.err.printf( "%s|server thread running\n", getClass().getName());
      }
      for(;;) {
         try {
            _isStarted.countDown();
            connectTo( _server.accept()).sendRegistry();
         }
         catch( final ClosedChannelException x ) {
            break;
         }
         catch( final IOException e ) {
            e.printStackTrace();
         }
      }
      if( Registry.LOG ) {
         System.err.printf( "%s|server thread ended\n", getClass().getName());
      }
   }
}
