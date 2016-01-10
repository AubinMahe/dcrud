package org.hpms.mw.dcrud;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.StandardProtocolFamily;
import java.nio.channels.DatagramChannel;

final class UDPNetworkReceiver extends DatagramNetworkReceiver {

   UDPNetworkReceiver( AbstractParticipant participant, InetSocketAddress other, boolean dumpReceivedBuffer ) throws IOException {
      super( participant, DatagramChannel
         .open(( other.getAddress().getAddress().length > 4 )
                  ? StandardProtocolFamily.INET6
                  : StandardProtocolFamily.INET              )
         .bind( other                                        ),
         dumpReceivedBuffer
      );
      System.out.printf( "receiving from %s\n", other );
   }
}
