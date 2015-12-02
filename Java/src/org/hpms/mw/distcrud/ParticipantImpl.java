package org.hpms.mw.distcrud;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.NetworkInterface;
import java.net.ProtocolFamily;
import java.net.StandardProtocolFamily;
import java.net.StandardSocketOptions;
import java.nio.ByteBuffer;
import java.nio.channels.DatagramChannel;
import java.util.HashMap;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;
import java.util.TreeMap;
import java.util.function.Supplier;

final class ParticipantImpl implements IParticipant {

   static final byte[] SIGNATURE = { 'H','P','M','S'};

   public static final int FRAME_TYPE_SIZE = 1;
   public static final int SIZE_SIZE       = 4;
   public static final int GUID_SIZE       = 1 + 1 + 1 + 4;
   public static final int CLASS_ID_SIZE   = 1 + 1 + 1 + 1;
   public static final int HEADER_SIZE     =
      SIGNATURE.length
      + FRAME_TYPE_SIZE
      + SIZE_SIZE
      + GUID_SIZE
      + CLASS_ID_SIZE;
   public static final int PAYLOAD_SIZE  =  ( 64*1024 ) - HEADER_SIZE;

   private final ByteBuffer              _header     = ByteBuffer.allocate( HEADER_SIZE );
   private final ByteBuffer              _payload    = ByteBuffer.allocate( PAYLOAD_SIZE );
   private final ByteBuffer              _message    = ByteBuffer.allocate( 64*1024 );
   /*   */ final Cache[]                 _caches     = new Cache[256];
   /*   */ final Dispatcher              _dispatcher = new Dispatcher( this );
   /*   */ final Map<Integer, ICallback> _callbacks  = new HashMap<>();
   /*   */ final byte                    _publisherId;
   private final Map<ClassID,
      Supplier<Shareable>>               _factories  = new TreeMap<>();
   private final InetSocketAddress       _target;
   private final DatagramChannel         _out;
   private /* */ int                     _callId = 1;

   ParticipantImpl( byte publisherId, InetSocketAddress group, NetworkInterface  intrfc ) throws IOException {
      final ProtocolFamily family =
         ( group.getAddress().getAddress().length > 4 )
            ? StandardProtocolFamily.INET6
            : StandardProtocolFamily.INET;
      _publisherId = publisherId;
      _target      = group;
      _out         = DatagramChannel
         .open     ( family )
//         .setOption( StandardSocketOptions.SO_REUSEADDR, true )
//         .bind     ( group )
         .setOption( StandardSocketOptions.IP_MULTICAST_IF, intrfc )
      ;
   }

   Shareable newInstance( ClassID classId, ByteBuffer frame ) {
      synchronized( _factories ) {
         final Supplier<Shareable> factory = _factories.get( classId );
         if( factory == null ) {
            return null;
         }
         final Shareable item = factory.get();
         item.unserialize( frame );
         return item;
      }
   }

   @Override
   public void registerClass( ClassID id, Supplier<Shareable> factory ) {
      synchronized( _factories ) {
         _factories.put( id, factory );
      }
   }

   @Override
   public ICache createCache() {
      synchronized( _caches ) {
         final Cache cache = new Cache( this );
         assert cache._cacheId > 0;
         return _caches[cache._cacheId-1] = cache;
      }
   }

   @Override
   public ICache getCache( byte cacheId ) {
      assert cacheId > 0;
      synchronized( _caches ) {
         return _caches[cacheId-1];
      }
   }

   @Override
   public IDispatcher getDispatcher() {
      return _dispatcher;
   }

   private void pushCreateOrUpdateItem( Shareable item ) throws IOException {
      synchronized( _out ){
         _payload.clear();
         item.serialize( _payload );
         _payload.flip();
         final int size = _payload.remaining();
         _header.clear();
         _header.put( SIGNATURE );
         _header.put((byte)FrameType.DATA_CREATE_OR_UPDATE.ordinal());
         _header.putInt( size );
         item._id   .serialize( _header );
         item._class.serialize( _header );
         _header.flip();
         _message.clear();
         _message.put( _header  );
         _message.put( _payload );
         _message.flip();
         _out.send( _message, _target );
      }
   }

   void publishUpdated( Set<? extends Shareable> updated ) throws IOException {
      for( final Shareable item : updated ) {
         pushCreateOrUpdateItem( item );
      }
   }

   private void pushDeleteItem( Shareable item ) throws IOException {
      synchronized( _out ){
         _header.put( SIGNATURE );
         _header.put((byte)FrameType.DATA_DELETE.ordinal());
         item._id.serialize( _header );
         _header.flip();
         _out.send( _header, _target );
      }
   }

   void publishDeleted( Set<? extends Shareable> deleted ) throws IOException {
      for( final Shareable item : deleted ) {
         pushDeleteItem( item );
      }
   }

   void call( String intrfcName, String opName, Map<String, Object> in, int callId ) throws IOException {
      synchronized( _out ){
         _message.clear();
         _header.clear();
         _header.put( SIGNATURE );
         _header.put((byte)FrameType.OPERATION.ordinal());
         _header.putInt( in.size());
         _header.flip();
         _message.put( _header );
         SerializerHelper.putString( intrfcName, _message );
         SerializerHelper.putString( opName    , _message );
         _message.putInt( callId );
         for( final Entry<String, Object> e : in.entrySet()) {
            _payload.clear();
            final String name  = e.getKey();
            final Object value = e.getValue();
            SerializerHelper.putString( name, _payload );
            switch( name ) {
            case "@queue":
               _payload.put(((Integer)value ).byteValue());
               break;
            case "@mode":
               _payload.put((byte)((IRequired.CallMode)value ).ordinal());
               break;
            default:
               if( value == null ) {
                  ClassID.NullClassID.serialize( _payload );
               }
               else if( value instanceof Shareable ) {
                  final Shareable item = (Shareable)value;
                  item._class.serialize( _payload );
                  item.serialize( _payload );
               }
               else if( value instanceof Byte ) {
                  ClassID.ByteClassID.serialize( _payload );
                  _payload.put((Byte)value );
               }
               else if( value instanceof Short ) {
                  ClassID.ShortClassID.serialize( _payload );
                  _payload.putShort((Short)value );
               }
               else if( value instanceof Integer ) {
                  ClassID.IntegerClassID.serialize( _payload );
                  _payload.putInt((Integer)value );
               }
               else if( value instanceof Long ) {
                  ClassID.LongClassID.serialize( _payload );
                  _payload.putLong((Long)value );
               }
               else if( value instanceof Float ) {
                  ClassID.FloatClassID.serialize( _payload );
                  _payload.putFloat((Float)value );
               }
               else if( value instanceof Double ) {
                  ClassID.DoubleClassID.serialize( _payload );
                  _payload.putDouble((Double)value );
               }
               else if( value instanceof ClassID ) {
                  ClassID.ClassIDClassID.serialize( _payload );
                  ((ClassID)value).serialize( _payload );
               }
               else if( value instanceof GUID ) {
                  ClassID.GUIDClassID.serialize( _payload );
                  ((GUID)value).serialize( _payload );
               }
               else {
                  throw new IllegalArgumentException(
                     name + " is of type " + value.getClass().getName() +
                     " which isn't null, primitive nor derived from " + Shareable.class.getName());
               }
               break;
            }
            _payload.flip();
            _message.put( _payload );
         }
         _message.flip();
         _out.send( _message, _target );
      }
   }

   int call( String intrfcName, String opName, Map<String, Object> in, ICallback callback ) throws IOException {
      call( intrfcName, opName, in, _callId );
      if( callback != null ) {
         _callbacks.put( _callId, callback );
         return _callId++;
      }
      return 0;
   }
}
