package tests.registry;

import java.nio.ByteBuffer;

final class Topic implements Comparable<Topic> {

   final int _value;

   public Topic( int value ) {
      _value = value;
   }

   void serialize( ByteBuffer to ) {
      to.putInt( _value );
   }

   static Topic unserialize( ByteBuffer from ) {
      return new Topic( from.getInt());
   }

   @Override
   public int compareTo( Topic right ) {
      return _value - right._value;
   }

   @Override
   public String toString() {
      return String.format( "0x%08X", _value );
   }
}
