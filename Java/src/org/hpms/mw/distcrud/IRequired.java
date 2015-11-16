package org.hpms.mw.distcrud;

import java.io.IOException;
import java.util.Map;
import java.util.function.BiConsumer;

public interface IRequired {

   void call( String opName ) throws IOException;

   void call( String opName, Map<String, Shareable> arguments ) throws IOException;

   int call(
      String                                      opName,
      Map<String, Shareable>                      arguments,
      BiConsumer<Integer, Map<String, Shareable>> callback  ) throws IOException;
}
