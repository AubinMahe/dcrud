package org.hpms.mw.distcrud;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.NetworkInterface;
import java.util.function.Supplier;

public interface IParticipant {

   void listen( NetworkInterface via, InetSocketAddress...others ) throws IOException;

   void registerLocalFactory ( ClassID id, Supplier<Shareable> factory );

   void registerRemoteFactory( ClassID id, ICRUD factory );

   ICache getDefaultCache();

   ICache createCache();

   ICache getCache( byte ID );

   IDispatcher getDispatcher();
}
