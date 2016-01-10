package org.hpms.mw.dcrud;

import java.nio.ByteBuffer;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Map.Entry;

import org.hpms.mw.dcrud.ClassID.Type;

public class Arguments {

   public enum CallMode {
      SYNCHRONOUS,
      ASYNCHRONOUS_DEFERRED,
      ASYNCHRONOUS_IMMEDIATE,
   }

   public static final byte     NON_URGENT_QUEUE = -128; // 0xFF
   public static final byte     NORMAL_QUEUE     =  127; // 0x7F
   public static final byte     URGENT_QUEUE     =    0; // 0x00
   public static final byte     DEFAULT_QUEUE     =  Arguments.NORMAL_QUEUE;
   public static final CallMode DEFAULT_CALL_MODE = CallMode.ASYNCHRONOUS_DEFERRED;

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
      final String name = "@mode";
      _args .put( name, mode );
      _types.put( name, ClassID.Type.CALL_MODE );
   }

   public void setQueue( byte queue ) {
      final String name = "@queue";
      _args .put( name, queue );
      _types.put( name, ClassID.Type.QUEUE_INDEX );
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
            case BOOLEAN    : SerializerHelper.putBoolean((Boolean)value, message );  break;
            case BYTE       : message.put                ((Byte   )value );           break;
            case SHORT      : message.putShort           ((Short  )value );           break;
            case INTEGER    : message.putInt             ((Integer)value );           break;
            case LONG       : message.putLong            ((Long   )value );           break;
            case FLOAT      : message.putFloat           ((Float  )value );           break;
            case DOUBLE     : message.putDouble          ((Double )value );           break;
            case STRING     : SerializerHelper.putString ((String )value, message );  break;
            case CLASS_ID   : ((ClassID)value).serialize( message );                  break;
            case GUID       : ((GUID   )value).serialize( message );                  break;
            case CALL_MODE  : message.put        ((byte)((CallMode)value).ordinal()); break;
            case QUEUE_INDEX: message.put                ((Byte   )value );           break;
            case SHAREABLE  : /* Already handled before this switch. */               break;
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
}
