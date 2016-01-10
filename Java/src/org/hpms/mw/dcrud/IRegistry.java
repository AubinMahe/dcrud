package org.hpms.mw.dcrud;

import java.net.InetSocketAddress;
import java.util.Set;

public interface IRegistry {

   Set<InetSocketAddress> getParticipants();
   Set<InetSocketAddress> getClients  ( String intrfc   );
   Set<InetSocketAddress> getConsumers( String dataname );
}
