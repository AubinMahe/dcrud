package tests.registry;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SocketChannel;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;

final class Sender extends Thread {

   private final SocketChannel          _channel;
   private final Registry               _registry;
   private final BlockingQueue<Boolean> _signal = new ArrayBlockingQueue<>( 10 );

   Sender( SocketChannel channel, Registry registry ) {
      _channel  = channel;
      _registry = registry;
      setName( String.format( "%s[%s ==> %s]",
         getClass().getName(),
         channel.socket().getLocalSocketAddress ().toString(),
         channel.socket().getRemoteSocketAddress().toString() ));
      setDaemon( true );
      start();
   }

   boolean is( InetSocketAddress addr ) throws IOException {
      return _channel.getLocalAddress().equals( addr );
   }

   boolean send() {
      return _signal.offer( Boolean.TRUE );
   }

   @Override
   public void run() {
      System.err.println( getName() + "|sender thread running" );
      while( _channel.isOpen()) {
         try {
            _signal.take();
            _registry.writeTo( _channel );
            System.err.println( getName() + "|registry sent" );
         }
         catch( final InterruptedException x ) {
            break;
         }
         catch( final ClosedChannelException x ) {
            break;
         }
         catch( final Throwable t ) {
            t.printStackTrace();
         }
      }
      System.err.println( getName() + "|sender thread ended" );
   }

   public void close() throws IOException {
      _channel.shutdownOutput();
      interrupt();
   }

   @Override
   public String toString() {
      return getName();
   }
}
