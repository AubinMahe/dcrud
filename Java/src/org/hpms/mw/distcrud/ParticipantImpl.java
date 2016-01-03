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

final class ParticipantImpl implements IParticipant, IProtocol {

   private final ByteBuffer                        _header          = ByteBuffer.allocate( HEADER_SIZE );
   private final ByteBuffer                        _payload         = ByteBuffer.allocate( PAYLOAD_SIZE );
   private final ByteBuffer                        _message         = ByteBuffer.allocate( 64*1024 );
   private final Cache[]                           _caches          = new Cache[256];
   private final Dispatcher                        _dispatcher      = new Dispatcher( this );
   private final Map<ClassID, Supplier<Shareable>> _localFactories  = new TreeMap<>();
   private final Map<ClassID, ICRUD >              _remoteFactories = new TreeMap<>();
   private final Map<Integer, ICallback>           _callbacks       = new HashMap<>();
   private final int                               _publisherId;
   private final InetSocketAddress                 _target;
   private final DatagramChannel                   _out;
   private /* */ int                               _callId     = 1;
   private /* */ byte                              _cacheCount = 0;

   ParticipantImpl( int publisherId, InetSocketAddress group, NetworkInterface  intrfc ) throws IOException {
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

   int getPublisherId() {
      return _publisherId;
   }

   Shareable newInstance( ClassID classId, ByteBuffer frame ) {
      synchronized( _localFactories ) {
         final Supplier<Shareable> factory = _localFactories.get( classId );
         if( factory == null ) {
            return null;
         }
         final Shareable item = factory.get();
         item.unserialize( frame );
         return item;
      }
   }

   @Override
   public void registerLocalFactory( ClassID id, Supplier<Shareable> factory ) {
      synchronized( _localFactories ) {
         _localFactories.put( id, factory );
      }
   }

   @Override
   public void registerRemoteFactory( ClassID id, ICRUD publisher ) {
      synchronized( _remoteFactories ) {
         _remoteFactories.put( id, publisher );
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
      synchronized( _out ) {
         final byte count = ( args == null ) ? 0 : (byte)args.getCount();
         _message.clear();
         _message        .put      ( SIGNATURE );
         _message        .put      ((byte)FrameType.OPERATION.ordinal());
         SerializerHelper.putString( intrfcName, _message );
         SerializerHelper.putString( opName    , _message );
         _message        .putInt   ( callId );
         _message        .put      ( count );
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
         for( int i = 0; i < _cacheCount; ++i ) {
            _caches[i].deleteFromNetwork( id );
         }
      }
   }

   void dataUpdate( ByteBuffer frame, int payloadSize ) {
      synchronized( _caches ) {
         for( int i = 0; i < _cacheCount; ++i ) {
            final byte[] copy = new byte[payloadSize];
            System.arraycopy( frame.array(), frame.position(), copy, 0, payloadSize );
            final ByteBuffer fragment = ByteBuffer.wrap( copy );
//            Dump.dump( fragment );
            _caches[i].updateFromNetwork( fragment );
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
      final ICRUD publisher = _remoteFactories.get( clsId );
      if( publisher == null ) {
         return false;
      }
      publisher.create( how );
      return true;
   }

   public boolean update( GUID id, Arguments how ) throws IOException {
      for( int i = 0; i < _cacheCount; ++i ) {
         final Shareable item = _caches[i].read( id );
         if( item != null ) {
            final ICRUD publisher = _remoteFactories.get( item._class );
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
      for( int i = 0; i < _cacheCount; ++i ) {
         final Shareable item = _caches[i].read( id );
         if( item != null ) {
            final ICRUD publisher = _remoteFactories.get( item._class );
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
