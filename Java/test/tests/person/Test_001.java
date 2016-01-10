package tests.person;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.NetworkInterface;

import tests.Settings;
import tests.StaticRegistry;
import tests.Tests;

public class Test_001 implements Settings {

   public static void main( String[] args ) throws IOException {
      final NetworkInterface via = NetworkInterface.getByName( INTERFACE );
      if( Tests.checkNetworkInterface( via )) {
         final InetSocketAddress p1 = new InetSocketAddress( MCAST_ADDR, PORT   );
         final InetSocketAddress p2 = new InetSocketAddress( MCAST_ADDR, PORT+1 );
         final StaticRegistry registry = new StaticRegistry();
         new Publisher ( 1, p1, via, registry );
         new Subscriber( 2, p2, via, registry );
      }
   }
}
