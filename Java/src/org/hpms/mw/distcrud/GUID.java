package org.hpms.mw.distcrud;

import java.nio.ByteBuffer;

public final class GUID implements Comparable<GUID> {

   static GUID unserialize( ByteBuffer buffer ) {
      final GUID id = new GUID();
      id._publisher = buffer.getShort();
      id._cache     = buffer.get();
      id._instance  = buffer.getInt();
      assert id._publisher > 0;
      assert id._cache     > 0;
      assert id._instance  > 0;
      return id;
   }

   short _publisher;
   byte  _cache;
   int   _instance;

   boolean isShared() {
      return _instance > 0;
   }

   void set( GUID id ) {
      _publisher = id._publisher;
      _cache     = id._cache;
      _instance  = id._instance;
      assert id._publisher > 0;
      assert id._cache     > 0;
      assert id._instance  > 0;
   }

   @Override
   public String toString() {
      return String.format( "Instance-%02X-%02X-%04X", _publisher, _cache, _instance );
   }

   @Override
   public int compareTo( GUID right ) {
      int delta = 0;
      if( delta == 0 ) {
         delta = this._publisher - right._publisher;
      }
      if( delta == 0 ) {
         delta = this._cache     - right._cache;
      }
      if( delta == 0 ) {
         delta = this._instance  - right._instance;
      }
      return delta;
   }

   public void serialize( ByteBuffer buffer ) {
      buffer.putShort( _publisher );
      buffer.put     ( _cache     );
      buffer.putInt  ( _instance  );
   }
}
