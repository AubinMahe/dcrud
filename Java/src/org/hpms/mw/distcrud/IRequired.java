package org.hpms.mw.distcrud;

import java.io.IOException;

public interface IRequired {

   void call( String opName ) throws IOException;

   void call( String opName, Arguments arguments ) throws IOException;

   void call( String opName, Arguments arguments, ICallback callback ) throws IOException;
}
