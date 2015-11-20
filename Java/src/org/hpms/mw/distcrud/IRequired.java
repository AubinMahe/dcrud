package org.hpms.mw.distcrud;

import java.io.IOException;
import java.util.Map;

public interface IRequired {

   int VERY_URGENT_QUEUE =   0;
   int URGENT_QUEUE      =  50;
   int DEFAULT_QUEUE     = 100;
   int NON_URGENT_QUEUE  = 255;

   enum CallMode {
      SYNCHRONOUS,
      ASYNCHRONOUS_DEFERRED,
      ASYNCHRONOUS_IMMEDIATE,
   }

   int call( String opName, Map<String, Object> arguments, ICallback callback ) throws IOException;
}
