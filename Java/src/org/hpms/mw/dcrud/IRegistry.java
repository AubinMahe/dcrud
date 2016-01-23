package org.hpms.mw.dcrud;

import java.net.InetSocketAddress;
import java.util.Set;

public interface IRegistry {

   Set<InetSocketAddress> getParticipants();
   Set<InetSocketAddress> getClients  ( String service );
   Set<InetSocketAddress> getConsumers( String topic );
}
