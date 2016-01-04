package tests.person;

import java.io.IOException;
import java.net.InetSocketAddress;

import tests.Settings;

public class Test_011 implements Settings {

   public static void main( String[] args ) throws IOException {
      final InetSocketAddress p1 = new InetSocketAddress( UDP_ADDR, PORT   );
      final InetSocketAddress p2 = new InetSocketAddress( UDP_ADDR, PORT+1 );
      new UDPPublisher ( 1, p1, p2 );
      new UDPSubscriber( 2, p2, p1 );
   }
}
