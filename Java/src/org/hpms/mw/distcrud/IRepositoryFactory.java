package org.hpms.mw.distcrud;

import java.util.function.Function;

public interface IRepositoryFactory {

   public <T extends Shareable>
   IRepository<T> createRepository( String topic, Function<GUID, ? extends Shareable> factory );
}
