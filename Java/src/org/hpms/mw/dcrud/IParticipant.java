package org.hpms.mw.dcrud;

import java.io.IOException;
import java.net.NetworkInterface;
import java.util.function.Supplier;

public interface IParticipant {

   void listen( NetworkInterface via, IRegistry registry, boolean dumpReceivedBuffer ) throws IOException;

   void listen( IRegistry registry, boolean dumpReceivedBuffer ) throws IOException;

   void registerLocalFactory ( ClassID id, Supplier<Shareable> factory );

   void registerRemoteFactory( ClassID id, ICRUD factory );

   ICache getDefaultCache();

   ICache createCache();

   ICache getCache( byte ID );

   IDispatcher getDispatcher();
}
