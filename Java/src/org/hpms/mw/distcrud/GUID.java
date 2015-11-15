package org.hpms.mw.distcrud;

import java.nio.ByteBuffer;

public final class GUID implements Comparable<GUID> {

   static GUID unserialize( ByteBuffer buffer ) {
      final GUID id = new GUID();
      id._platform = buffer.get();
      id._exec     = buffer.get();
      id._cache    = buffer.get();
      id._instance = buffer.getInt();
      assert id._platform > 0;
      assert id._exec     > 0;
      assert id._cache    > 0;
      return id;
   }

   byte _platform;
   byte _exec;
   byte _cache;
   int  _instance;

   boolean isShared() {
      return _instance > 0;
   }

   void set( GUID id ) {
      _platform = id._platform;
      _exec     = id._exec;
      _cache    = id._cache;
      _instance = id._instance;
      assert id._platform > 0;
      assert id._exec     > 0;
      assert id._cache    > 0;
   }

   @Override
   public String toString() {
      return String.format( "Instance-%02X-%02X-%02X-%04X",
         _platform, _exec, _cache, _instance );
   }

   @Override
   public int compareTo( GUID right ) {
      int delta = 0;
      if( delta == 0 ) {
         delta = this._platform - right._platform;
      }
      if( delta == 0 ) {
         delta = this._exec     - right._exec;
      }
      if( delta == 0 ) {
         delta = this._cache    - right._cache;
      }
      if( delta == 0 ) {
         delta = this._instance - right._instance;
      }
      return delta;
   }

   public void serialize( ByteBuffer buffer ) {
      buffer.put   ( _platform );
      buffer.put   ( _exec     );
      buffer.put   ( _cache    );
      buffer.putInt( _instance );
   }
}
