package org.hpms.mw.dcrud;

import java.nio.ByteBuffer;

public class Topic implements Comparable<Topic> {

   private final int _value;

   Topic( int value ) {
      _value = value;
   }

   @Override
   public int compareTo( Topic right ) {
      return this._value - right._value;
   }

   @Override
   public boolean equals( Object right ) {
      return this._value == ((Topic)right)._value;
   }

   @Override
   public int hashCode() {
      return Integer.hashCode( _value );
   }

   public void serialize( ByteBuffer to ) {
      to.putInt( _value );
   }

   public static Topic unserialize( ByteBuffer from ) {
      final int value = from.getInt();
      return new Topic( value );
   }
}
