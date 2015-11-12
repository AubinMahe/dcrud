package org.hpms.mw.distcrud;

import java.io.IOException;
import java.util.Map;
import java.util.function.Predicate;

public interface IRepository<T extends Shareable> {

   void         create ( T            item, int classId );
   T            read   ( GUID         id    );
   void         update ( T            item  );
   void         delete ( T            item  );
   Map<GUID, T> select ( Predicate<T> query );
   void         publish() throws IOException;
   void         refresh();
}
