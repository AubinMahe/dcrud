package org.hpms.mw.dcrud;

import java.io.IOException;
import java.util.Collection;
import java.util.Set;
import java.util.function.Predicate;

public interface ICache {

   void                    setOwnership( boolean enabled );
   boolean                 owns        ( GUID id );
   Status                  create      ( Shareable item );
   <T extends Shareable> T read        ( GUID id );
   Status                  update      ( Shareable item );
   Status                  delete      ( Shareable item );
   Collection<Shareable>   values      ();
   Set<Shareable>          select      ( Predicate<Shareable> query );
   void                    publish     () throws IOException;
   void                    subscribe   ( Topic topic );
   void                    refresh     ();
}
