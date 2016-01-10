package org.hpms.mw.dcrud;

import java.nio.ByteBuffer;

public final class ClassID implements Comparable<ClassID> {

   enum Type {
      NULL,
      BYTE,
      BOOLEAN,
      SHORT,
      INTEGER,
      LONG,
      FLOAT,
      DOUBLE,
      STRING,
      CLASS_ID,
      GUID,
      CALL_MODE,
      QUEUE_INDEX,
      SHAREABLE,
   }

   static ClassID unserialize( ByteBuffer buffer ) {
      final byte package_1 = buffer.get();
      final byte package_2 = buffer.get();
      final byte package_3 = buffer.get();
      final byte classId   = buffer.get();
      return new ClassID( package_1, package_2, package_3, classId );
   }

   final byte _package_1;
   final byte _package_2;
   final byte _package_3;
   final byte _class;

   public ClassID( byte package_1, byte package_2, byte package_3, byte classId ) {
      if(( package_1 > 0 ) || ( package_2 > 0 ) || ( package_3 > 0 )) {
         if( ! (( package_1 > 0 ) && ( package_2 > 0 ) && ( package_3 > 0 ))) {
            throw new IllegalStateException( "Bad package value, must be > 0" );
         }
      }
      if( classId == 0 ) {
         throw new IllegalStateException( "Bad class value, must be > 0" );
      }
      _package_1 = package_1;
      _package_2 = package_2;
      _package_3 = package_3;
      _class     = classId;
   }

   public Type getType() {
      if( ( _package_1 > 0 ) || ( _package_2 > 0 ) || ( _package_3 > 0 ) ) {
         return Type.SHAREABLE;
      }
      return Type.values()[_class];
   }

   @Override
   public String toString() {
      return String.format( "%02X-%02X-%02X-%02X", _package_1, _package_2, _package_3, _class );
   }

   @Override
   public int compareTo( ClassID right ) {
      int delta = 0;
      if( delta == 0 ) {
         delta = this._package_1 - right._package_1;
      }
      if( delta == 0 ) {
         delta = this._package_2 - right._package_2;
      }
      if( delta == 0 ) {
         delta = this._package_3 - right._package_3;
      }
      if( delta == 0 ) {
         delta = this._class     - right._class;
      }
      return delta;
   }

   static void serialize( Type type, ByteBuffer buffer ) {
      buffer.put((byte)0 );
      buffer.put((byte)0 );
      buffer.put((byte)0 );
      buffer.put((byte)type.ordinal());
   }

   public void serialize( ByteBuffer buffer ) {
      buffer.put( _package_1 );
      buffer.put( _package_2 );
      buffer.put( _package_3 );
      buffer.put( _class );
   }
}
