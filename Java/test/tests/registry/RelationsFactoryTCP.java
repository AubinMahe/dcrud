package tests.registry;

import java.io.IOException;
import java.net.BindException;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;
import java.util.function.Consumer;

public class RelationsFactoryTCP implements IRelationsFactory, Runnable {

   private final ServerSocketChannel  _server = ServerSocketChannel.open();
   private final int                  _bootPort;
   private final Registry             _registry;
   private final Consumer<ByteBuffer> _dataConsumer;
   private /* */ InetSocketAddress    _thisEndPoint;

   RelationsFactoryTCP(
      String               host,
      int                  bootPort,
      Registry             registry,
      Consumer<ByteBuffer> dataConsumer ) throws IOException
   {
      _bootPort     = bootPort;
      _registry     = registry;
      _dataConsumer = dataConsumer;
      for( int port = _bootPort; true; ++port ) {
         try {
            _server.bind( new InetSocketAddress( host, port ));
            if( Registry.LOG ) {
               System.err.printf( "%s|server listening on port %d\n",
                  getClass().getName(), port );
            }
            _thisEndPoint = (InetSocketAddress)_server.getLocalAddress();
            _registry.addEndPoint( _thisEndPoint );
            new Thread( this ).start();
            if( port != _bootPort ) {
               connectTo( new InetSocketAddress( host, _bootPort ));
            }
            break;
         }
         catch( final BindException x ) {/**/}
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
      for( int err = 0; err < 3; ++err ) {
         try {
            connectTo( _server.accept()).sendRegistry();
            err = 0;
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
