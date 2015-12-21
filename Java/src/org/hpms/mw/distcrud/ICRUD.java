package org.hpms.mw.distcrud;

import java.io.IOException;

public interface ICRUD {

   void create( Arguments how                 ) throws IOException;
   void update( Shareable what, Arguments how ) throws IOException;
   void delete( Shareable what                ) throws IOException;
}
