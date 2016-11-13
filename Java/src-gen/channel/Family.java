package channel;

public class Family implements org.hpms.mw.channel.ISerializable<Family> {

   public static final class Factory implements org.hpms.mw.channel.IFactory<Family> {

      @Override
      public int getClassID() {
         return 3;
      }

      @Override
      public Family create() {
         final Family family = new Family();
         return family;
      }

      @Override
      public void encode( Family family, org.hpms.mw.channel.CoDec encoder ) {
         encoder.putValue( family._parent1  );
         encoder.putReference( family._parent2  );
         encoder.putSet( family._children );
      }

      @Override
      public void decode( Family family, org.hpms.mw.channel.CoDec decoder ) {
         decoder.getValue( family._parent1 );
         family._parent2 = decoder.getReference();
         decoder.getSet( family._children );
      }
   }

   private static final org.hpms.mw.channel.IFactory<Family> TheFactory = new Factory();

   final Person _parent1 = new Person();
   /* */ Person _parent2;
   final java.util.Set<Person> _children = new java.util.LinkedHashSet<>();

   public Family() {/**/}

   public Family( Person parent1 ) {
      _parent1.set( parent1 );
   }

   @Override
   public org.hpms.mw.channel.IFactory<Family> getFactory() {
      return TheFactory;
   }

   public void set( Family family ) {
      _parent1.set( family._parent1 );
      _parent2 = family._parent2;
      setChildren( family._children );
   }

   public void setParent1( Person parent1 ) {
      _parent1.set( parent1 );
   }

   public Person getParent1() {
      return _parent1;
   }

   public void setParent2( Person parent2 ) {
      _parent2 = parent2;
   }

   public Person getParent2() {
      return _parent2;
   }

   public void setChildren( java.util.Set<Person> children ) {
      _children.clear();
      _children.addAll( children );
   }

   public java.util.Set<Person> getChildren() {
      return _children;
   }

   @Override
   public String toString() {
      return
         '{' + _parent1 .toString() +
         ',' + _parent2 .toString() +
         ',' + _children.toString() +
         '}';
   }
}
