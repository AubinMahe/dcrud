package org.hpms.mw.distcrud;

import java.io.IOException;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Map;

public class RepositoryFactoryBuilder {

   private static final Map<InetAddress, IRepositoryFactory> _Groups = new HashMap<>();

   public static IRepositoryFactory join( String addr, String intrfc, int port, int id ) throws IOException {
      final NetworkInterface netwkIntrfc = NetworkInterface.getByName( intrfc );
      if( netwkIntrfc == null ) {
         for( final Enumeration<NetworkInterface> e = NetworkInterface.getNetworkInterfaces();
            e.hasMoreElements(); )
         {
            final NetworkInterface ni = e.nextElement();
            if( ni.isUp()) {
               System.err.println( ni + " ==> virtual: " + ni.isVirtual());
            }
         }
         throw new IllegalArgumentException( intrfc + " isn't a valid network interface!" );
      }
      if( ! netwkIntrfc.isUp()) {
         throw new IllegalStateException( netwkIntrfc + " must be up!" );
      }
      final InetAddress address = InetAddress.getByName( addr );
      if( ! address.isMulticastAddress()) {
         throw new IllegalArgumentException( address + " isn't a valid multicast address!" );
      }
      assert port > 0;
      IRepositoryFactory factory = _Groups.get( address );
      if( factory == null ) {
         _Groups.put( address, factory = new Repositories( address, netwkIntrfc, port, id ));
      }
      return factory;
   }
}
