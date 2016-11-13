package tests.person;

import java.nio.ByteBuffer;
import java.time.LocalDate;

import org.hpms.mw.dcrud.ClassID;
import org.hpms.mw.dcrud.Shareable;

final class Person extends Shareable {

   public static final ClassID CLASS_ID = new ClassID((byte)1, (byte)1, (byte)1, (byte)1 );

   private String    _forname;
   private String    _name;
   private LocalDate _birthdate;

   public Person() {
      super( CLASS_ID );
   }

   public Person( String forname, String name, LocalDate birthdate ) {
      super( CLASS_ID );
      _forname   = forname;
      _name      = name;
      _birthdate = birthdate;
   }

   public void update( String forname, String name, String birthdate ) {
      if( forname != null ) {
         _forname   = forname;
      }
      if( name != null ) {
         _name      = name;
      }
      if( birthdate != null ) {
         _birthdate = LocalDate.parse( birthdate );
      }
   }

   public String getForname() {
      return _forname;
   }

   public String getName() {
      return _name;
   }

   public LocalDate getBirthDate() {
      return _birthdate;
   }

   @Override
   public void serialize( ByteBuffer target ) {
      org.hpms.util.SerializerHelper.putString   ( _forname  , target );
      org.hpms.util.SerializerHelper.putString   ( _name     , target );
      tests            .SerializerHelper.putLocalDate( _birthdate, target );
   }

   @Override
   public void unserialize( ByteBuffer source ) {
      _forname   = org.hpms.util.SerializerHelper.getString   ( source );
      _name      = org.hpms.util.SerializerHelper.getString   ( source );
      _birthdate = tests            .SerializerHelper.getLocalDate( source );
   }

   @Override
   public String toString() {
      return _forname + ' ' + _name + ' ' + _birthdate;
   }
}
