package tests.registry;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.ByteChannel;
import java.util.function.Consumer;

public class Relation implements Consumer<ByteBuffer> {

   private static final short REGISTRY    = 1;
   private static final short CMD_OR_DATA = 2;

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
      _payload.putShort( REGISTRY );
      _registry.serializeTo( _payload );
      _sender.send( _payload );
   }

   void sendCommandOrData() {
      _payload.clear();
      _payload.putShort( CMD_OR_DATA );
      _sender.send( _payload );
   }

   public void close() throws IOException {
      _sender  .close();
      _receiver.close();
   }

   @Override
   public void accept( ByteBuffer payload ) {
      final short msgId = payload.getShort();
      switch( msgId ) {
      case REGISTRY   : _registry.merge( payload ); break;
      case CMD_OR_DATA: _dataConsumer.accept( payload ); break;
      default: System.err.printf( "%s.accept|Unexpected message: %d\n", msgId ); break;
      }
   }
}
