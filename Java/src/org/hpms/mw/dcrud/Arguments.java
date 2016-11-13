package org.hpms.mw.dcrud;

import java.nio.ByteBuffer;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Map.Entry;

import org.hpms.mw.dcrud.ClassID.Type;
import org.hpms.util.SerializerHelper;

public class Arguments {

   public enum CallMode {
      SYNCHRONOUS,
      ASYNCHRONOUS_DEFERRED,
      ASYNCHRONOUS_IMMEDIATE,
   }

   public static final byte     NON_URGENT_QUEUE  = -128; // 255
   public static final byte     NORMAL_QUEUE      =  100;
   public static final byte     URGENT_QUEUE      =   50;
   public static final byte     VERY_URGENT_QUEUE =    0;
   public static final byte     DEFAULT_QUEUE     = Arguments.NORMAL_QUEUE;
   public static final CallMode DEFAULT_CALL_MODE = CallMode.ASYNCHRONOUS_DEFERRED;

   private /* */ CallMode                  _mode  = CallMode.SYNCHRONOUS;
   private /* */ byte                      _queue = DEFAULT_QUEUE;
   private final Map<String, Object>       _args  = new LinkedHashMap<>();
   private final Map<String, ClassID.Type> _types = new LinkedHashMap<>();

   public Arguments() {
      setMode ( DEFAULT_CALL_MODE );
      setQueue( DEFAULT_QUEUE );
   }

   public void clear() {
      _args.clear();
   }

   public boolean isEmpty() {
      return _args.isEmpty();
   }

   public void putNull( String name ) {
      _args .put( name, null );
      _types.put( name, ClassID.Type.NULL );
   }

   public void put( String name, byte value ) {
      _args .put( name, value );
      _types.put( name, ClassID.Type.BYTE );
   }

   public void put( String name, boolean value ) {
      _args .put( name, value );
      _types.put( name, ClassID.Type.BOOLEAN );
   }

   public void put( String name, short value ) {
      _args .put( name, value );
      _types.put( name, ClassID.Type.SHORT );
   }

   public void put( String name, int value ) {
      _args .put( name, value );
      _types.put( name, ClassID.Type.INTEGER );
   }

   public void put( String name, long value ) {
      _args .put( name, value );
      _types.put( name, ClassID.Type.LONG );
   }

   public void put( String name, float value ) {
      _args .put( name, value );
      _types.put( name, ClassID.Type.FLOAT );
   }

   public void put( String name, double value ) {
      _args .put( name, value );
      _types.put( name, ClassID.Type.DOUBLE );
   }

   public void put( String name, String value ) {
      _args .put( name, value );
      _types.put( name, ClassID.Type.STRING );
   }

   public void put( String name, ClassID value ) {
      _args .put( name, value );
      _types.put( name, ClassID.Type.CLASS_ID );
   }

   public void put( String name, GUID value ) {
      _args .put( name, value );
      _types.put( name, ClassID.Type.GUID );
   }

   public void put( String name, Shareable value ) {
      _args .put( name, value );
      _types.put( name, ClassID.Type.SHAREABLE );
   }

   public void setMode( CallMode mode ) {
      _mode = mode;
   }

   public void setQueue( byte queue ) {
      _queue = queue;
   }

   public boolean isNull( String name ) {
      return _types.get( name ) == ClassID.Type.NULL;
   }

   public boolean getBoolean( String name ) {
      final Byte b = get( name );
      return 0 != b.byteValue();
   }

   @SuppressWarnings("unchecked")
   public <T> T get( String name ) {
      return (T)_args.get( name );
   }

   public void serialize( ByteBuffer message ) {
      message.put((byte)_mode.ordinal());
      message.put(      _queue );
      for( final Entry<String, Object> e : _args.entrySet()) {
         final String       name  = e.getKey();
         final Object       value = e.getValue();
         final ClassID.Type type  = _types.get( name );
         SerializerHelper.putString( name, message );
         if( type == Type.SHAREABLE ) {
            final Shareable item = (Shareable)value;
            item._class.serialize( message );
            item.serialize( message );
         }
         else {
            ClassID.serialize( type, message );
            switch( type ) {
            case NULL       : /* No data */ break;
            case BOOLEAN    : SerializerHelper.putBoolean((Boolean)value, message );    break;
            case CHAR       : message.put       ((byte)((Character)value).charValue()); break;
            case BYTE       : message.put                ((Byte   )value );             break;
            case SHORT      : message.putShort           ((Short  )value );             break;
            case INTEGER    : message.putInt             ((Integer)value );             break;
            case LONG       : message.putLong            ((Long   )value );             break;
            case FLOAT      : message.putFloat           ((Float  )value );             break;
            case DOUBLE     : message.putDouble          ((Double )value );             break;
            case STRING     : SerializerHelper.putString ((String )value, message );    break;
            case CLASS_ID   : ((ClassID)value).serialize( message );                    break;
            case GUID       : ((GUID   )value).serialize( message );                    break;
            case SHAREABLE  : /* Already handled before this switch. */                 break;
            case UNSIGNED_INTEGER:
            case UNSIGNED_LONG:
            case UNSIGNED_SHORT:
            default:
               throw new IllegalArgumentException(
                  name + " is of type " + value.getClass().getName() +
                  " which isn't null, primitive nor derived from " + Shareable.class.getName());
            }
         }
      }
   }

   public int getCount() {
      return _args.size();
   }

   @Override
   public String toString() {
      final StringBuilder message = new StringBuilder();
      message.append( "   mode: " );
      message.append( _mode.name());
      message.append( '\n' );
      message.append( "   queue: " );
      message.append( _queue );
      message.append( '\n' );
      message.append( "   pairs:\n" );
      for( final Entry<String, Object> e : _args.entrySet()) {
         final String name  = e.getKey();
         final Object value = e.getValue();
         final ClassID.Type type = _types.get( name );
         message.append( "      " );
         message.append( name );
         message.append( " ==> (" );
         message.append( type );
         message.append( ") " );
         if( type == Type.STRING ) {
            final String s = (String)value;
            final byte[] bytes = s.getBytes();
            message.append( "String, length = " );
            message.append( bytes.length );
            message.append( ":\n" );
            for( int i = 0; i < bytes.length; ++i ) {
               message.append( '\t' );
               message.append( i );
               message.append( ": 0x" );
               message.append( Integer.toHexString( bytes[i] ));
               message.append( '\n' );
            }
         }
         else {
            message.append( value.toString());
         }
         message.append( '\n' );
      }
      return message.toString();
   }
}
