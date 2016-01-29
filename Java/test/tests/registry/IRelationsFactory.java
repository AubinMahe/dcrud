package tests.registry;

import java.io.IOException;
import java.net.InetSocketAddress;

interface IRelationsFactory {

   void connectTo( InetSocketAddress endPoint ) throws IOException;

   InetSocketAddress getLocalEndPoint();
}
