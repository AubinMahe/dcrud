package tests.registry;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.ReadableByteChannel;
import java.util.function.Consumer;

final class Receiver extends Thread {

   private final ReadableByteChannel  _channel;
   private final Consumer<ByteBuffer> _consumer;
   private final ByteBuffer           _header = ByteBuffer.allocate( Integer.BYTES );
   private final String               _channelId;

   public Receiver( ReadableByteChannel channel, String channelId, Consumer<ByteBuffer> consumer ) {
      _channel   = channel;
      _channelId = channelId;
      _consumer  = consumer;
      setName( String.format( "%s[%s]", getClass().getName(), _channelId ));
      setDaemon( true );
      start();
   }

   private ByteBuffer read() throws IOException {
      _header.clear();
      while( _header.position() < Integer.BYTES ) {
         if( _channel.read( _header ) < 0 ) {
            return null;
         }
      }
      _header.flip();
      final int        length  = _header.getInt();
      final ByteBuffer payload = ByteBuffer.allocate( length );
      while( payload.position() < length ) {
         if( _channel.read( payload ) < 0 ) {
            return null;
         }
      }
      payload.flip();
      return payload;
   }

   @Override
   public void run() {
      if( Registry.LOG ) {
         System.err.printf( "%s.run|running %s\n", getClass().getName(), _channelId );
      }
      while( _channel.isOpen()) {
         try {
            final ByteBuffer payload = read();
            if( payload != null ) {
               if( Registry.LOG ) {
                  System.err.printf( "%s.run|%d bytes read on %s\n", getClass().getName(),
                     payload.remaining(), _channelId );
               }
               _consumer.accept( payload );
            }
         }
         catch( final ClosedChannelException x ) {
            break;
         }
         catch( final Throwable t ) {
            t.printStackTrace();
         }
      }
      if( Registry.LOG ) {
         System.err.printf( "%s.run|ended %s\n", getClass().getName(), _channelId );
      }
   }

   public void close() throws IOException {
      _channel.close();
   }
}
