package org.hpms.mw.distcrud;

import java.io.IOException;
import java.util.Map;
import java.util.function.BiConsumer;

public interface IRequired {

   void enqueue( String opName ) throws IOException;

   void execute( String opName ) throws IOException;

   void enqueue( String opName, Map<String, Object> arguments ) throws IOException;

   void execute( String opName, Map<String, Object> arguments ) throws IOException;

   int  enqueue(
      String                                   opName,
      Map<String, Object>                      arguments,
      BiConsumer<Integer, Map<String, Object>> callback  ) throws IOException;

   int  execute(
      String                                   opName,
      Map<String, Object>                      arguments,
      BiConsumer<Integer, Map<String, Object>> callback  ) throws IOException;
}
