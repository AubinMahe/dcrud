package tests.registry;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.WritableByteChannel;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;

final class Sender extends Thread {

   private final WritableByteChannel       _channel;
   private final BlockingQueue<ByteBuffer> _msgQueue = new ArrayBlockingQueue<>( 10 );
   private final ByteBuffer                _header   = ByteBuffer.allocate( Integer.BYTES );

   Sender( WritableByteChannel channel, String channelId ) {
      _channel = channel;
      setName( String.format( "%s[%s]", getClass().getName(), channelId ));
      setDaemon( true );
      start();
   }

   boolean send( ByteBuffer frame ) {
      return _msgQueue.offer( frame );
   }

   private void write( ByteBuffer payload ) throws IOException {
      _header.clear();
      _header.putInt( payload.remaining());
      _header.flip();
      while( _header.hasRemaining()) {
         _channel.write( _header );
      }
      if( payload.position() > 0 ) {
         payload.flip();
      }
      while( payload.hasRemaining()) {
         _channel.write( payload );
      }
   }

   @Override
   public void run() {
      if( Registry.LOG ) {
         System.err.printf( "%s|sender thread running\n", getName());
      }
      while( _channel.isOpen()) {
         try {
            final ByteBuffer payload = _msgQueue.take();
            write( payload );
            if( Registry.LOG ) {
               System.err.printf( "%s|registry sent\n", getName());
            }
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
      if( Registry.LOG ) {
         System.err.printf( "%s|sender thread ended\n", getName());
      }
   }

   public void close() throws IOException {
      _channel.close();
      interrupt();
   }

   @Override
   public String toString() {
      return getName();
   }
}
