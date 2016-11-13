package channel;

import java.time.LocalDate;

public class Person implements org.hpms.mw.channel.ISerializable<Person> {

   public static final class Factory implements org.hpms.mw.channel.IFactory<Person> {

      @Override
      public int getClassID() {
         return 2;
      }

      @Override
      public Person create() {
         return new Person();
      }

      @Override
      public void encode( Person person, org.hpms.mw.channel.CoDec encoder ) {
         encoder.putString( person._forname );
         encoder.putString( person._name );
         encoder.putString( person._birthdate.toString());
      }

      @Override
      public void decode( Person person, org.hpms.mw.channel.CoDec decoder ) {
         person._forname   = decoder.getString();
         person._name      = decoder.getString();
         person._birthdate = LocalDate.parse( decoder.getString());
      }
   }

   private static final Factory TheFactory = new Factory();

   String    _forname;
   String    _name;
   LocalDate _birthdate;

   public Person() {/**/}

   public Person(
      String forname,
      String name,
      java.time.LocalDate birthdate )
   {
      _forname = forname;
      _name = name;
      _birthdate = birthdate;
   }

   @Override
   public org.hpms.mw.channel.IFactory<Person> getFactory() {
      return TheFactory;
   }

   public void set( Person person ) {
      _forname = person._forname;
      _name = person._name;
      _birthdate = person._birthdate;
   }

   public void setForname( String forname ) {
      _forname = forname;
   }

   public String getForname() {
      return _forname;
   }

   public void setName( String name ) {
      _name = name;
   }

   public String getName() {
      return _name;
   }

   public void setBirthdate( LocalDate birthdate ) {
      _birthdate = birthdate;
   }

   public LocalDate getBirthdate() {
      return _birthdate;
   }

   @Override
   public String toString() {
      return
         '{' + _forname   +
         ',' + _name      +
         ',' + _birthdate +
         '}';
   }
}
