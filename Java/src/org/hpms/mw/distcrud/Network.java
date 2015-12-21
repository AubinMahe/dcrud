package org.hpms.mw.distcrud;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.NetworkInterface;

public class Network {

   public static IParticipant join( byte id, InetSocketAddress addr, NetworkInterface intrfc ) throws IOException {
      return new ParticipantImpl( id, addr, intrfc );
   }
}
