package org.hpms.mw.dcrud;

import java.nio.ByteBuffer;

public final class GUID implements Comparable<GUID> {

   static GUID unserialize( ByteBuffer buffer ) {
      final GUID id = new GUID();
      id._publisher = buffer.getInt();
      id._instance  = buffer.getInt();
      assert id._publisher > 0;
      assert id._instance  > 0;
      return id;
   }

   int _publisher;
   int _instance;

   boolean isShared() {
      return _instance > 0;
   }

   void set( GUID id ) {
      _publisher = id._publisher;
      _instance  = id._instance;
      assert id._publisher > 0;
      assert id._instance  > 0;
   }

   @Override
   public String toString() {
      return String.format( "%08X-%08X", _publisher, _instance );
   }

   @Override
   public int compareTo( GUID right ) {
      int delta = 0;
      if( delta == 0 ) {
         delta = this._publisher - right._publisher;
      }
      if( delta == 0 ) {
         delta = this._instance  - right._instance;
      }
      return delta;
   }

   public void serialize( ByteBuffer buffer ) {
      buffer.putInt( _publisher );
      buffer.putInt( _instance  );
   }
}
