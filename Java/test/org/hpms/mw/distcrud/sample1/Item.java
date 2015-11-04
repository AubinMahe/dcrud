package org.hpms.mw.distcrud.sample1;

import java.nio.ByteBuffer;
import java.time.LocalDate;

import org.hpms.mw.distcrud.SerializerHelper;
import org.hpms.mw.distcrud.Shareable;

final class Item extends Shareable {

   public static final int ITEM_CLASS = 42;

   private String    _name;
   private String    _forname;
   private LocalDate _birth;
   private int       _count;

   public Item() {
      super( ITEM_CLASS );
   }

   public Item( String name, String forname, LocalDate birth, int count ) {
      super( ITEM_CLASS );
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
   public void set( Shareable source ) {
      final Item src = (Item)source;
      this._name    = src._name;
      this._forname = src._forname;
      this._birth   = src._birth;
      this._count   = src._count;
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
}
