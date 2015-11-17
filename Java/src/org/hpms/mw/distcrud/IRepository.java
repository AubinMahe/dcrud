package org.hpms.mw.distcrud;

import java.io.IOException;
import java.util.Set;
import java.util.function.Predicate;

public interface IRepository {

   byte           getId();
   void           ownership( boolean enabled );
   boolean        matches  ( GUID    id );
   Status         create   ( Shareable            item  );
   <T extends Shareable>
   T              read     ( GUID                 id    );
   Status         update   ( Shareable            item  );
   Status         delete   ( Shareable            item  );
   Set<Shareable> select   ( Predicate<Shareable> query );
   void           publish  () throws IOException;
   void           subscribe( ClassID id );
   void           refresh  ();
}
