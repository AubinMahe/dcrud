package org.hpms.mw.distcrud.samples.person;

import java.nio.ByteBuffer;
import java.time.LocalDate;

import org.hpms.mw.distcrud.ClassID;
import org.hpms.mw.distcrud.SerializerHelper;
import org.hpms.mw.distcrud.Shareable;

final class Item extends Shareable {

   public static final ClassID CLASS_ID = new ClassID((byte)1, (byte)1, (byte)1, (byte)1 );

   private String    _name;
   private String    _forname;
   private LocalDate _birth;
   private int       _count;

   public Item() {
      super( CLASS_ID );
   }

   public Item( String name, String forname, LocalDate birth, int count ) {
      super( CLASS_ID );
      _name    = name;
      _forname = forname;
      _birth   = birth;
      _count   = count;
   }

   public String getName() {
      return _name;
   }

   public String getForname() {
      return _forname;
   }

   public LocalDate getBirth() {
      return _birth;
   }

   public int getCount() {
      return _count;
   }

   @Override
   public void serialize( ByteBuffer target ) {
      SerializerHelper.putString   ( _name   , target );
      SerializerHelper.putString   ( _forname, target );
      SerializerHelper.putLocalDate( _birth  , target );
      target.putInt( _count );
   }

   @Override
   public void unserialize( ByteBuffer source ) {
      _name    = SerializerHelper.getString   ( source );
      _forname = SerializerHelper.getString   ( source );
      _birth   = SerializerHelper.getLocalDate( source );
      _count   = source.getInt();
   }

   @Override
   public String toString() {
      return _forname + ' ' + _name + ' ' + _birth;
   }
}
