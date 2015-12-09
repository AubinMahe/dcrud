package org.hpms.mw.distcrud;

import java.util.function.Supplier;

public interface IParticipant {

   void registerClass( ClassID id, Supplier<Shareable> factory );

   ICache createCache();

   ICache getCache( byte ID );

   IDispatcher getDispatcher();
}
