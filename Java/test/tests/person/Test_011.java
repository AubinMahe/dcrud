package tests.person;

import java.io.IOException;
import java.net.InetSocketAddress;

import org.hpms.mw.dcrud.IRegistry;

import tests.Settings;
import tests.StaticRegistry;

public class Test_011 implements Settings {

   public static void main( String[] args ) throws IOException {
      final IRegistry registry = new StaticRegistry();
      final InetSocketAddress p1 = new InetSocketAddress( UDP_ADDR, PORT   );
      final InetSocketAddress p2 = new InetSocketAddress( UDP_ADDR, PORT+1 );
      new UDPPublisher ( 1, p1, registry );
      new UDPSubscriber( 2, p2, registry );
   }
}
