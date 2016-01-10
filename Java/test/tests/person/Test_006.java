package tests.person;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.NetworkInterface;

import tests.Settings;
import tests.StaticRegistry;
import tests.Tests;

public class Test_006 implements Settings {

   public static void main( String[] args ) throws IOException {
      final NetworkInterface via = NetworkInterface.getByName( INTERFACE );
      if( Tests.checkNetworkInterface( via )) {
         new Publisher( 1, new InetSocketAddress( MCAST_ADDR, PORT ), via, new StaticRegistry());
      }
   }
}
