package org.hpms.mw.dcrud;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.NetworkInterface;
import java.util.Enumeration;

public class Network {

   public static boolean dumpReceivedBuffer = false;
   public static boolean recordPerformance  = false;

   public static IParticipant join( int id, InetSocketAddress addr, NetworkInterface intrfc ) throws IOException {
      return new MulticastParticipant( id, addr, intrfc );
   }

   public static IParticipant join( int id, InetSocketAddress addr ) throws IOException {
      if( addr.getAddress().isMulticastAddress()) {
         NetworkInterface intrfc = null;
         for( final Enumeration<NetworkInterface> e = NetworkInterface.getNetworkInterfaces();
              e.hasMoreElements(); )
         {
            final NetworkInterface candidate = e.nextElement();
            if( ! candidate.isLoopback()
               && candidate.isUp()
               && candidate.supportsMulticast())
            {
               intrfc = candidate;
            }
         }
         if( intrfc == null ) {
            throw new IllegalStateException(
               "No network up, non loopback, multicast capable interface found" );
         }
         return new MulticastParticipant( id, addr, intrfc );
      }
      return new UDPParticipant( id, addr );
   }
}
