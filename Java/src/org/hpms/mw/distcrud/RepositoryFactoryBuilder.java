package org.hpms.mw.distcrud;

import java.io.IOException;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Map;

public class RepositoryFactoryBuilder {

   private static final Map<InetAddress, IRepositoryFactory> _Groups = new HashMap<>();

   public static IRepositoryFactory join( String addr, String intrfc, int port ) throws IOException {
      final NetworkInterface netwkIntrfc = NetworkInterface.getByName( intrfc );
      if( netwkIntrfc == null ) {
         System.err.println( intrfc + " isn't a valid network interface!" );
         for( final Enumeration<NetworkInterface> e = NetworkInterface.getNetworkInterfaces();
            e.hasMoreElements(); )
         {
            final NetworkInterface ni = e.nextElement();
            if( ni.isUp()) {
               System.err.println( ni + " ==> virtual: " + ni.isVirtual());
            }
         }
         return null;
      }
      if( ! netwkIntrfc.isUp()) {
         System.err.println( netwkIntrfc + " must be up!" );
         return null;
      }
      final InetAddress address = InetAddress.getByName( addr );
      if( ! address.isMulticastAddress()) {
         System.err.println( address + " isn't a valid multicast address!" );
         return null;
      }
      assert port > 0;
      IRepositoryFactory factory = _Groups.get( address );
      if( factory == null ) {
         _Groups.put( address, factory = new Repositories( address, netwkIntrfc, port ));
      }
      return factory;
   }
}
