package tests.registry;

import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.NetworkInterface;
import java.nio.ByteBuffer;
import java.util.function.Consumer;

public class RelationsFactoryUDP implements IRelationsFactory {

   @SuppressWarnings("unused")
   public RelationsFactoryUDP( NetworkInterface intrfc, InetAddress host, int bootPort, Registry registry, Consumer<ByteBuffer> dataConsumer ) {
      // TODO Auto-generated constructor stub
   }



   @Override
   public void connectTo(
      InetSocketAddress endPoint ) throws IOException {
      // TODO Auto-generated method stub

   }



   @Override
   public InetSocketAddress getLocalEndPoint() {
      // TODO Auto-generated method stub
      return null;
   }

}
