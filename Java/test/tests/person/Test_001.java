package tests.person;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.NetworkInterface;

import tests.Tests;

public class Test_001 {

   static final String INTERFACE  = "eth0";
   static final String MCAST_ADDR = "224.0.0.3";
   static final int    PORT       = 2416;

   public static void main( String[] args ) throws IOException {
      final NetworkInterface via = NetworkInterface.getByName( INTERFACE );
      if( Tests.checkNetworkInterface( via )) {
         final InetSocketAddress p1 = new InetSocketAddress( MCAST_ADDR, PORT   );
         final InetSocketAddress p2 = new InetSocketAddress( MCAST_ADDR, PORT+1 );
         new Publisher ((byte)1, p1, via, p2 );
         new Subscriber((byte)2, p2, via, p1 );
      }
   }
}
