package org.hpms.mw.distcrud;

import java.nio.ByteBuffer;

public final class ClassID implements Comparable<ClassID> {

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
      _package_1 = package_1;
      _package_2 = package_2;
      _package_3 = package_3;
      _class     = classId;
   }

   boolean isShared() {
      return _class > 0;
   }

   @Override
   public String toString() {
      return String.format( "Class-%02X-%02X-%02X-%02X",
         _package_1, _package_2, _package_3, _class );
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

   public void serialize( ByteBuffer buffer ) {
      buffer.put( _package_1 );
      buffer.put( _package_2 );
      buffer.put( _package_3 );
      buffer.put( _class );
   }
}
