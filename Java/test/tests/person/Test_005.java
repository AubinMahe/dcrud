package tests.person;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.NetworkInterface;

import org.hpms.mw.dcrud.Network;

import tests.Settings;
import tests.StaticRegistry;
import tests.Tests;

public class Test_005 implements Settings {

   public static void main( String[] args ) throws IOException {
      Network.dumpReceivedBuffer =
         ( args.length > 0 )&& args[0].equals( "--dump-received-buffer=true" );
      final NetworkInterface via = NetworkInterface.getByName( INTERFACE );
      if( Tests.checkNetworkInterface( via )) {
         new Subscriber( 2, new InetSocketAddress( MCAST_ADDR, PORT ), via, new StaticRegistry());
      }
   }
}
