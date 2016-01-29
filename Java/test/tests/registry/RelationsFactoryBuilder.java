package tests.registry;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.function.Consumer;

public class RelationsFactoryBuilder {

   private static RelationsFactoryTCP _tcp;

   public static IRelationsFactory createTCP(
      String               host,
      int                  bootPort,
      Registry             registry,
      Consumer<ByteBuffer> dataConsumer ) throws IOException
   {
      if( _tcp == null ) {
         _tcp = new RelationsFactoryTCP( host, bootPort, registry, dataConsumer );
      }
      return _tcp;
   }
}
