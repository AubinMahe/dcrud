package org.hpms.mw.distcrud;

public final class GUID implements Comparable<GUID> {

   public final int _source;
   public final int _instance;

   GUID( int sourceId, int instance ) {
      _source   = sourceId;
      _instance = instance;
   }

   public boolean matchClass( int classId ) {
      return _source == classId;
   }

   @Override
   public String toString() {
      return String.format( "%04X-%04X", _source, _instance );
   }

   @Override
   public int compareTo( GUID right ) {
      int delta = this._source  - right._source;
      if( delta == 0 ) {
         delta = this._instance - right._instance;
      }
      return delta;
   }
}
