package org.hpms.mw.distcrud;

import java.io.IOException;
import java.util.Map;
import java.util.function.Predicate;

public interface IRepository<T extends Shareable> {

   public void         create ( T            item  );
   public T            read   ( GUID         id    );
   public void         update ( T            item  );
   public void         delete ( T            item  );
   public Map<GUID, T> select ( Predicate<T> query );
   public boolean      isProducer();
   public boolean      isConsumer();
   public void         publish() throws IOException;
   public void         refresh();
}
