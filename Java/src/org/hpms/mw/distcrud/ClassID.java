package org.hpms.mw.distcrud;

import java.nio.ByteBuffer;

public final class ClassID implements Comparable<ClassID> {

   static ClassID NullClassID    = new ClassID();
   static ClassID ByteClassID    = new ClassID((byte)0);
   static ClassID BooleanClassID = new ClassID( false );
   static ClassID ShortClassID   = new ClassID((short)0);
   static ClassID IntegerClassID = new ClassID( 0 );
   static ClassID LongClassID    = new ClassID( 0L );
   static ClassID FloatClassID   = new ClassID( 0f );
   static ClassID DoubleClassID  = new ClassID( 0.0 );
   static ClassID StringClassID  = new ClassID( "" );
   static ClassID ClassIDClassID = new ClassID( NullClassID );
   static ClassID GUIDClassID    = new ClassID( new GUID());


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

   private ClassID() {
      _package_1 = 0;
      _package_2 = 0;
      _package_3 = 0;
      _class     = 0;
   }

   private ClassID( @SuppressWarnings("unused") byte v ) {
      _package_1 = 0;
      _package_2 = 0;
      _package_3 = 0;
      _class     = 1;
   }

   private ClassID( @SuppressWarnings("unused") boolean v ) {
      _package_1 = 0;
      _package_2 = 0;
      _package_3 = 0;
      _class     = 2;
   }

   private ClassID( @SuppressWarnings("unused") short v ) {
      _package_1 = 0;
      _package_2 = 0;
      _package_3 = 0;
      _class     = 3;
   }

   private ClassID( @SuppressWarnings("unused") int v ) {
      _package_1 = 0;
      _package_2 = 0;
      _package_3 = 0;
      _class     = 4;
   }

   private ClassID( @SuppressWarnings("unused") long v ) {
      _package_1 = 0;
      _package_2 = 0;
      _package_3 = 0;
      _class     = 5;
   }

   private ClassID( @SuppressWarnings("unused") float v ) {
      _package_1 = 0;
      _package_2 = 0;
      _package_3 = 0;
      _class     = 6;
   }

   private ClassID( @SuppressWarnings("unused") double v ) {
      _package_1 = 0;
      _package_2 = 0;
      _package_3 = 0;
      _class     = 7;
   }

   private ClassID( @SuppressWarnings("unused") String v ) {
      _package_1 = 0;
      _package_2 = 0;
      _package_3 = 0;
      _class     = 8;
   }

   private ClassID( @SuppressWarnings("unused") ClassID v ) {
      _package_1 = 0;
      _package_2 = 0;
      _package_3 = 0;
      _class     = 9;
   }

   private ClassID( @SuppressWarnings("unused") GUID v ) {
      _package_1 = 0;
      _package_2 = 0;
      _package_3 = 0;
      _class     = 10;
   }

   public ClassID( byte package_1, byte package_2, byte package_3, byte classId ) {
      assert package_1 > 0;
      assert package_2 > 0;
      assert package_3 > 0;
      assert classId   > 0;
      _package_1 = package_1;
      _package_2 = package_2;
      _package_3 = package_3;
      _class     = classId;
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
