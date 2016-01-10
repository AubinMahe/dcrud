package tests;

import java.net.InetSocketAddress;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import org.hpms.mw.dcrud.IRegistry;

public final class StaticRegistry implements IRegistry, Settings {

   private final Set<InetSocketAddress>              _participants = new HashSet<>();
   private final Map<String, Set<InetSocketAddress>> _clients      = new HashMap<>();
   private final Map<String, Set<InetSocketAddress>> _consumers    = new HashMap<>();

   public StaticRegistry() {
      _participants.add( new InetSocketAddress( MCAST_ADDR, PORT   ));
      _participants.add( new InetSocketAddress( MCAST_ADDR, PORT+1 ));
      _participants.add( new InetSocketAddress( MCAST_ADDR, PORT+2 ));
      _participants.add( new InetSocketAddress( MCAST_ADDR, PORT+3 ));
   }

   @Override
   public Set<InetSocketAddress> getParticipants() {
      return _participants;
   }

   @Override
   public Set<InetSocketAddress> getClients( String interfaceName ) {
      return _clients.get( interfaceName );
   }

   @Override
   public Set<InetSocketAddress> getConsumers( String dataName ) {
      return _consumers.get( dataName );
   }
}
