package org.hpms.mw.dcrud;

import java.io.IOException;

public interface IRequired {

   void call( String opName ) throws IOException;

   void call( String opName, Arguments arguments ) throws IOException;

   void call( String opName, Arguments arguments, ICallback callback ) throws IOException;
}
