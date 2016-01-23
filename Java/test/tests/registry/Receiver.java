package tests.registry;

import java.io.IOException;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SocketChannel;

final class Receiver extends Thread {

   private final SocketChannel _channel;
   private final Registry      _registry;

   public Receiver( SocketChannel channel, Registry registry ) {
      _channel  = channel;
      _registry = registry;
      setName( String.format( "%s[%s <== %s]",
         getClass().getName(),
         channel.socket().getLocalSocketAddress ().toString(),
         channel.socket().getRemoteSocketAddress().toString() ));
      setDaemon( true );
      start();
   }

   @Override
   public void run() {
      System.err.println( getName() + "|receiver thread running" );
      while( _channel.isOpen()) {
         try {
            if( ! _registry.merge( _channel )) {
               break;
            }
            System.err.println( getName() + "|registry merged" );
         }
         catch( final ClosedChannelException x ) {
            break;
         }
         catch( final Throwable t ) {
            t.printStackTrace();
         }
      }
      System.err.println( getName() + "|receiver thread ended" );
   }

   public void close() throws IOException {
      _channel.shutdownInput();
   }
}
