package tests.registry;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.ByteChannel;
import java.util.function.Consumer;

public class Relation implements Consumer<ByteBuffer> {

   private static final byte REGISTRY    = 1;
   private static final byte CMD_OR_DATA = 2;

   final ByteBuffer           _payload = ByteBuffer.allocate( 64*1024 );
   final Registry             _registry;
   final Consumer<ByteBuffer> _dataConsumer;
   final Sender               _sender;
   final Receiver             _receiver;

   Relation(
      ByteChannel          channel,
      String               channelId,
      Registry             registry,
      Consumer<ByteBuffer> dataConsumer )
   {
      _registry     = registry;
      _dataConsumer = dataConsumer;
      _sender       = new Sender  ( channel, channelId );
      _receiver     = new Receiver( channel, channelId, this );
   }

   void sendRegistry() {
      _payload.clear();
      _payload.put( REGISTRY );
      _registry.serializeTo( _payload );
      _payload.flip();
      _sender.send( _payload );
   }

   void send( ByteBuffer commandOrData ) {
      _payload.clear();
      _payload.put( CMD_OR_DATA );
      if( commandOrData.position() > 0 ) {
         commandOrData.flip();
      }
      _payload.putInt( commandOrData.remaining());
      _payload.put( commandOrData );
      _sender.send( _payload );
   }

   public void close() throws IOException {
      _sender  .close();
      _receiver.close();
   }

   @Override
   public void accept( ByteBuffer payload ) {
      final byte msgType = payload.get();
      if( Registry.LOG ) {
         System.err.printf( "%s.accept|Received message: %d\n", getClass().getName(),
            msgType );
      }
      switch( msgType ) {
      case REGISTRY   : _registry.merge( payload ); break;
      case CMD_OR_DATA: _dataConsumer.accept( payload ); break;
      default:
         System.err.printf( "%s.accept|Unexpected message type: %d\n", getClass().getName(),
            msgType );
      break;
      }
   }
}
