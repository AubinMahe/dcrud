package org.hpms.mw.distcrud;

import java.nio.ByteBuffer;

public final class GUID implements Comparable<GUID> {

   static GUID unserialize( ByteBuffer buffer ) {
      final String topic    = SerializerHelper.getString( buffer );
      final int    classId  = buffer.getInt();
      final int    instance = buffer.getInt();
      return new GUID( topic, classId, instance );
   }

   public final String _topic;
   public final int    _class;
   public /* */ int    _instance;

   GUID( String topic, int classId, int instance ) {
      _topic    = topic;
      _class    = classId;
      _instance = instance;
   }

   public GUID( String topic, int classId ) {
      this( topic, classId, 0 );
   }

   boolean isShared() {
      return _instance > 0;
   }

   void setInstance( int instance ) {
      _instance = instance;
   }

   @Override
   public String toString() {
      return String.format( "%s.%04X-%04X", _topic, _class, _instance );
   }

   @Override
   public int compareTo( GUID right ) {
      int delta = this._topic.compareTo( right._topic );
      if( delta == 0 ) {
         delta = this._class - right._class;
      }
      if( delta == 0 ) {
         delta = this._instance - right._instance;
      }
      return delta;
   }

   public void serialize( ByteBuffer buffer ) {
      SerializerHelper.putString( _topic, buffer );
      buffer.putInt( _class );
      buffer.putInt( _instance );
   }
}
