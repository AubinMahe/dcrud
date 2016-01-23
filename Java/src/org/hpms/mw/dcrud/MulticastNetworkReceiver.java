package org.hpms.mw.dcrud;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.NetworkInterface;
import java.net.StandardProtocolFamily;
import java.net.StandardSocketOptions;
import java.nio.channels.DatagramChannel;

final class MulticastNetworkReceiver extends DatagramNetworkReceiver {

   MulticastNetworkReceiver(
      AbstractParticipant participant,
      InetSocketAddress   source,
      NetworkInterface    intrfc ) throws IOException
   {
      super( participant, DatagramChannel
         .open     (( source.getAddress().getAddress().length > 4 )
                        ? StandardProtocolFamily.INET6
                        : StandardProtocolFamily.INET              )
         .setOption( StandardSocketOptions.SO_REUSEADDR, true      )
         .bind     ( source                                        )
         .setOption( StandardSocketOptions.IP_MULTICAST_IF, intrfc )
      );
      _in.join( source.getAddress(), intrfc );
      System.out.printf( "receiving from %s, bound to %s\n", source, intrfc );
   }
}
