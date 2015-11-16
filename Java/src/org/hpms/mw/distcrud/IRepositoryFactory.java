package org.hpms.mw.distcrud;

import java.util.function.Supplier;

public interface IRepositoryFactory {

   void registerClass( ClassID id, Supplier<Shareable> factory );

   IRepository createRepository();

   IRepository getRepository( byte ID );

   IDispatcher getDispatcher();

   void run();
}
