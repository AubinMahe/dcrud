package tests.registry;

import java.io.IOException;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.nio.ByteBuffer;
import java.util.function.Consumer;

public class RelationsFactoryBuilder {

   private static IRelationsFactory _factory;

   public static IRelationsFactory createTCP(
      NetworkInterface     intrfc,
      InetAddress          registryServer,
      int                  registryPort,
      int                  bootPort,
      Registry             registry,
      Consumer<ByteBuffer> dataConsumer ) throws IOException
   {
      if( _factory == null ) {
         if( registryServer.isMulticastAddress()) {
            _factory = new RelationsFactoryUDP( intrfc, registryServer, bootPort, registry, dataConsumer );
         }
         else {
            _factory =
               new RelationsFactoryTCP(
                  intrfc,
                  registryServer, registryPort,
                  bootPort,
                  registry,
                  dataConsumer );
         }
      }
      return _factory;
   }
}
