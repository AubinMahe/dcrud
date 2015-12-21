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
import java.util.Set;
import java.util.TreeMap;
import java.util.function.Supplier;

enum FrameType {

   NO_OP,
   DATA_CREATE_OR_UPDATE,
   DATA_DELETE,
   OPERATION
}

final class ParticipantImpl implements IParticipant {

   static final byte[] SIGNATURE = {'D','C','R','U','D'};

   public static final int FRAME_TYPE_SIZE = 1;
   public static final int SIZE_SIZE       = 4;
   public static final int GUID_SIZE       = 4 + 4;
   public static final int CLASS_ID_SIZE   = 1 + 1 + 1 + 1;
   public static final int HEADER_SIZE     =
      SIGNATURE.length
      + FRAME_TYPE_SIZE
      + SIZE_SIZE
      + GUID_SIZE
      + CLASS_ID_SIZE;
   public static final int PAYLOAD_SIZE  =  ( 64*1024 ) - HEADER_SIZE;

   private final ByteBuffer                        _header     = ByteBuffer.allocate( HEADER_SIZE );
   private final ByteBuffer                        _payload    = ByteBuffer.allocate( PAYLOAD_SIZE );
   private final ByteBuffer                        _message    = ByteBuffer.allocate( 64*1024 );
   private final Cache[]                           _caches     = new Cache[256];
   private final Dispatcher                        _dispatcher = new Dispatcher( this );
   private final Map<ClassID, Supplier<Shareable>> _factories  = new TreeMap<>();
   private final Map<ClassID, ICRUD >              _publishers = new TreeMap<>();
   private final Map<Integer, ICallback>           _callbacks  = new HashMap<>();
   private final byte                              _publisherId;
   private final InetSocketAddress                 _target;
   private final DatagramChannel                   _out;
   private /* */ int                               _callId     = 1;
   private /* */ byte                              _cacheCount = 0;

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
      createCache();
      System.out.printf( "Sending to %s via interface %s\n", group, intrfc );
   }

   @Override
   public void listen( NetworkInterface via, InetSocketAddress...others ) throws IOException {
      for( final InetSocketAddress other : others ) {
         new NetworkReceiver( this, other, via );
      }
   }

   short getPublisherId() {
      return _publisherId;
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
   public void registerFactory( ClassID id, Supplier<Shareable> factory ) {
      synchronized( _factories ) {
         _factories.put( id, factory );
      }
   }

   @Override
   public void registerPublisher( ClassID id, ICRUD publisher ) {
      synchronized( _publishers ) {
         _publishers.put( id, publisher );
      }
   }

   @Override
   public ICache getDefaultCache() {
      synchronized( _caches ) {
         return _caches[0];
      }
   }

   @Override
   public ICache createCache() {
      synchronized( _caches ) {
         return _caches[_cacheCount++] = new Cache( this );
      }
   }

   @Override
   public ICache getCache( byte cacheIndex ) {
      synchronized( _caches ) {
         return _caches[cacheIndex];
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
         _header.clear();
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

   void call( String intrfcName, String opName, Arguments args, int callId ) throws IOException {
      synchronized( _out ){
         _message.clear();
         _header.clear();
         _header.put( SIGNATURE );
         _header.put((byte)FrameType.OPERATION.ordinal());
         _header.put( args == null ? 0 : (byte)args.getCount());
         _header.flip();
         _message.put( _header );
         SerializerHelper.putString( intrfcName, _message );
         SerializerHelper.putString( opName    , _message );
         _message.putInt( callId );
         if( args != null ) {
            args.serialize( _message );
         }
         _message.flip();
         _out.send( _message, _target );
      }
   }

   void call( String intrfcName, String opName, Arguments args, ICallback callback ) throws IOException {
      assert callback != null;
      call( intrfcName, opName, args, _callId );
      _callbacks.put( _callId++, callback );
   }

   void dataDelete( GUID id ) {
      synchronized( _caches ) {
         for( final Cache cache : _caches ) {
            if( cache == null ) {
               break;
            }
            cache.deleteFromNetwork( id );
         }
      }
   }

   void dataUpdate( ByteBuffer frame, int payloadSize ) {
      synchronized( _caches ) {
         for( final Cache cache : _caches ) {
            if( cache == null ) {
               break;
            }
            final byte[] copy = new byte[payloadSize];
            System.arraycopy( frame.array(), frame.position(), copy, 0, payloadSize );
            final ByteBuffer fragment = ByteBuffer.wrap( copy );
//            Dump.dump( fragment );
            cache.updateFromNetwork( fragment );
         }
      }
   }

   void callback( String intrfcName, String opName, Arguments args, int callId ) {
      final ICallback callback = _callbacks.get( callId );
      if( callback == null ) {
         System.err.printf( "Unknown Callback received: %s.%s, id: %d\n",
            intrfcName, opName, -callId );
      }
      else {
         callback.callback( intrfcName, opName, args );
      }
   }

   public boolean create( ClassID clsId, Arguments how ) throws IOException {
      if( clsId == null ) {
         return false;
      }
      final ICRUD publisher = _publishers.get( clsId );
      if( publisher == null ) {
         return false;
      }
      publisher.create( how );
      return true;
   }

   public boolean update( GUID id, Arguments how ) throws IOException {
      for( final Cache cache : _caches ) {
         if( cache == null ) {
            return false;
         }
         final Shareable item = cache.read( id );
         if( item != null ) {
            final ICRUD publisher = _publishers.get( item._class );
            if( publisher == null ) {
               return false;
            }
            publisher.update( item, how );
            return true;
         }
      }
      return false;
   }

   public boolean delete( GUID id ) throws IOException {
      for( final Cache cache : _caches ) {
         if( cache == null ) {
            return false;
         }
         final Shareable item = cache.read( id );
         if( item != null ) {
            final ICRUD publisher = _publishers.get( item._class );
            if( publisher == null ) {
               return false;
            }
            publisher.delete( item );
            return true;
         }
      }
      return false;
   }
}
