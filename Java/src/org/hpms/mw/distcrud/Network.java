package org.hpms.mw.distcrud;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.NetworkInterface;

public class Network {

   public static IParticipant join( int id, InetSocketAddress addr, NetworkInterface intrfc ) throws IOException {
      return new MulticastParticipant( id, addr, intrfc );
   }

   public static IParticipant join( int id, InetSocketAddress addr ) throws IOException {
      if( addr.getAddress().isMulticastAddress()) {
         return new MulticastParticipant( id, addr, NetworkInterface.getByIndex( 0 ));
      }
      return new UDPParticipant( id, addr );
   }
}
