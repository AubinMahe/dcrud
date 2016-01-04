package org.hpms.mw.distcrud;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.StandardProtocolFamily;
import java.nio.channels.DatagramChannel;

final class UDPNetworkReceiver extends DatagramNetworkReceiver {

   UDPNetworkReceiver( AbstractParticipant participant, InetSocketAddress other ) throws IOException {
      super( participant, DatagramChannel
         .open(( other.getAddress().getAddress().length > 4 )
                  ? StandardProtocolFamily.INET6
                  : StandardProtocolFamily.INET              )
         .bind( other                                        )
      );
      System.out.printf( "receiving from %s\n", other );
   }
}
