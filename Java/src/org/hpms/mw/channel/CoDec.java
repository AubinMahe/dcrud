package org.hpms.mw.channel;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.DatagramChannel;
import java.util.Set;

import org.hpms.util.SerializerHelper;

public class CoDec {

   private final ByteBuffer _buffer = ByteBuffer.allocate( 64*1024 );

   public void init( short id ) {
      _buffer.clear();
      _buffer.putShort( id );
   }

   public void putBoolean( boolean value ) {
      _buffer.put((byte)( value ? 1 : 0 ));
   }

   public boolean getBoolean() {
      final byte value = _buffer.get();
      return value != 0;
   }

   public void putByte( byte value ) {
      _buffer.putInt( value );
   }

   public byte getByte() {
      final byte value = _buffer.get();
      return value;
   }

   public void putShort( short value ) {
      _buffer.putShort( value );
   }

   public short getShort() {
      final short value = _buffer.getShort();
      return value;
   }

   public void putInt( int value ) {
      _buffer.putInt( value );
   }

   public int getInt() {
      final int value = _buffer.getInt();
      return value;
   }

   public void putLong( long value ) {
      _buffer.putLong( value );
   }

   public long getLong() {
      final long value = _buffer.getLong();
      return value;
   }

   public void putFloat( float value ) {
      _buffer.putFloat( value );
   }

   public float getFloat() {
      final float value = _buffer.getFloat();
      return value;
   }

   public void putDouble( double value ) {
      _buffer.putDouble( value );
   }

   public double getDouble() {
      final double value = _buffer.getDouble();
      return value;
   }

   public void putString( String value ) {
      SerializerHelper.putString( value, _buffer );
   }

   public String getString() {
      return SerializerHelper.getString( _buffer );
   }

   public <T extends ISerializable<T>> void putReference( T ref ) {
      if( ref == null ) {
         putInt( 0 );
      }
      else {
         final IFactory<T> factory = ref.getFactory();
         putInt( factory.getClassID());
         factory.encode( ref, this );
      }
   }

   public <T extends ISerializable<T>> T getReference() {
      final int classID = getInt();
      if( classID == 0 ) {
         return null;
      }
      final IFactory<T> itemFactory = Factories.get( classID );
      if( itemFactory == null ) {
         throw new IllegalStateException( String.format( "Unregistered factory: %d", classID ));
      }
      final T t = itemFactory.create();
      itemFactory.decode( t, this );
      return t;
   }

   public <T extends ISerializable<T>> void putValue( T value ) {
      final IFactory<T> factory = value.getFactory();
      factory.encode( value, this );
   }

   public <T extends ISerializable<T>> void getValue( T value ) {
      final IFactory<T> itemFactory = value.getFactory();
      itemFactory.decode( value, this );
   }

   public <T extends ISerializable<T>> void putArray( T[] array ) {
      for( final T t : array ) {
         putValue( t );
      }
   }

   public <T extends ISerializable<T>> T[] getArray( int classID, T[] array ) {
      final IFactory<T> itemFactory = Factories.get( classID );
      for( int i = 0; i < array.length; ++i ) {
         T t = array[i];
         if( t == null ) {
            t = array[i] = itemFactory.create();
         }
         itemFactory.decode( t, this );
      }
      return array;
   }

   public <T extends ISerializable<T>> void putSet( Set<T> set ) {
      _buffer.putInt( set.size());
      for( final T t : set ) {
         putReference( t );
      }
   }

   public <T extends ISerializable<T>> void getSet( Set<T> target ) {
      target.clear();
      for( int i = 0, length = _buffer.getInt(); i < length; ++i ) {
         final T t = getReference();
         target.add( t );
      }
   }

   ByteBuffer getBuffer() {
      return _buffer;
   }

   short receive( DatagramChannel channel ) throws IOException {
      _buffer.clear();
      channel.receive( _buffer );
      _buffer.flip();
      final short msgId = _buffer.getShort();
      return msgId;
   }

   public String dump() {
      final int pos = _buffer.position();
      _buffer.rewind();
      int col = 0;
      final StringBuffer ret = new StringBuffer( 1024 );
      final StringBuffer str = new StringBuffer( 16 );
      while( _buffer.hasRemaining()) {
         final byte b = _buffer.get();
         ret.append( String.format( "%02X ", b ));
         if( ( b > 20 ) && ( b < 128 ) ) {
            str.append((char)b );
         }
         else {
            str.append( '.' );
         }
         ++col;
         if( col == 8 ) {
            ret.append( "- " );
         }
         else if( col == 16 ) {
            ret.append( str );
            ret.append( '\n' );
            str.setLength( 0 );
            col = 0;
         }
      }
      if( col > 0 ) {
         for( int i = col; i < 16; ++i ) {
            ret.append( "   " );
            str.append( ' ' );
            if( i == 8 ) {
               ret.append( "- " );
            }
            else if( col == 16 ) {
               ret.append( str );
               ret.append( '\n' );
               str.setLength( 0 );
               col = 0;
            }
         }
         ret.append( str );
         ret.append( '\n' );
         str.setLength( 0 );
      }
      _buffer.position( pos );
      return ret.toString();
   }
}
