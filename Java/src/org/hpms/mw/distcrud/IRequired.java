package org.hpms.mw.distcrud;

import java.io.IOException;

public interface IRequired {

   byte URGENT_QUEUE     =    0; // 0x00
   byte NORMAL_QUEUE     =  127; // 0x7F
   byte NON_URGENT_QUEUE = -128; // 0xFF

   enum CallMode {
      SYNCHRONOUS,
      ASYNCHRONOUS_DEFERRED,
      ASYNCHRONOUS_IMMEDIATE,
   }

   byte     DEFAULT_QUEUE     =  NORMAL_QUEUE;
   CallMode DEFAULT_CALL_MODE = CallMode.ASYNCHRONOUS_DEFERRED;

   void call( String opName ) throws IOException;

   void call( String opName, Arguments arguments ) throws IOException;

   void call( String opName, Arguments arguments, ICallback callback ) throws IOException;
}
