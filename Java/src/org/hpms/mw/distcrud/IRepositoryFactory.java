package org.hpms.mw.distcrud;

import java.util.function.Function;

public interface IRepositoryFactory {

   public <T extends Shareable> IRepository<T> getRepository(
      int                                    classId,
      boolean                                owner,
      Function<Integer, ? extends Shareable> factory );
}
