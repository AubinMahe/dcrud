package org.hpms.mw.distcrud;

import java.nio.ByteBuffer;

public final class ClassID implements Comparable<ClassID> {

   enum PredefinedType {
      NullType,
      ByteType,
      BooleanType,
      ShortType,
      IntegerType,
      LongType,
      FloatType,
      DoubleType,
      StringType,
      ClassIDType,
      GUIDType,
   }

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
      _class     = (byte)PredefinedType.NullType.ordinal();
   }

   private ClassID( @SuppressWarnings("unused") byte v ) {
      _package_1 = 0;
      _package_2 = 0;
      _package_3 = 0;
      _class     = (byte)PredefinedType.ByteType.ordinal();
   }

   private ClassID( @SuppressWarnings("unused") boolean v ) {
      _package_1 = 0;
      _package_2 = 0;
      _package_3 = 0;
      _class     = (byte)PredefinedType.BooleanType.ordinal();
   }

   private ClassID( @SuppressWarnings("unused") short v ) {
      _package_1 = 0;
      _package_2 = 0;
      _package_3 = 0;
      _class     = (byte)PredefinedType.ShortType.ordinal();
   }

   private ClassID( @SuppressWarnings("unused") int v ) {
      _package_1 = 0;
      _package_2 = 0;
      _package_3 = 0;
      _class     = (byte)PredefinedType.IntegerType.ordinal();
   }

   private ClassID( @SuppressWarnings("unused") long v ) {
      _package_1 = 0;
      _package_2 = 0;
      _package_3 = 0;
      _class     = (byte)PredefinedType.LongType.ordinal();
   }

   private ClassID( @SuppressWarnings("unused") float v ) {
      _package_1 = 0;
      _package_2 = 0;
      _package_3 = 0;
      _class     = (byte)PredefinedType.FloatType.ordinal();
   }

   private ClassID( @SuppressWarnings("unused") double v ) {
      _package_1 = 0;
      _package_2 = 0;
      _package_3 = 0;
      _class     = (byte)PredefinedType.DoubleType.ordinal();
   }

   private ClassID( @SuppressWarnings("unused") String v ) {
      _package_1 = 0;
      _package_2 = 0;
      _package_3 = 0;
      _class     = (byte)PredefinedType.StringType.ordinal();
   }

   private ClassID( @SuppressWarnings("unused") ClassID v ) {
      _package_1 = 0;
      _package_2 = 0;
      _package_3 = 0;
      _class     = (byte)PredefinedType.ClassIDType.ordinal();
   }

   private ClassID( @SuppressWarnings("unused") GUID v ) {
      _package_1 = 0;
      _package_2 = 0;
      _package_3 = 0;
      _class     = (byte)PredefinedType.GUIDType.ordinal();
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

   public boolean isPredefined() {
      return ( _package_1 == 0 ) && ( _package_2 == 0 ) && ( _package_3 == 0 );
   }

   public PredefinedType getPredefinedTypeID() {
      return isPredefined() ? PredefinedType.values()[_class] : null;
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

   public void serialize( ByteBuffer buffer ) {
      buffer.put( _package_1 );
      buffer.put( _package_2 );
      buffer.put( _package_3 );
      buffer.put( _class );
   }
}
