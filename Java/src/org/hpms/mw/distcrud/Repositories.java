package org.hpms.mw.distcrud;

import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.NetworkInterface;
import java.net.ProtocolFamily;
import java.net.StandardProtocolFamily;
import java.net.StandardSocketOptions;
import java.nio.ByteBuffer;
import java.nio.channels.DatagramChannel;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;
import java.util.TreeMap;
import java.util.function.BiConsumer;
import java.util.function.Supplier;

final class Repositories implements IRepositoryFactory {

   private static final byte[] SIGNATURE = { 'D','I','S','T','C','R','U','D'};

   public static final int SIZE_SIZE     = 4;
   public static final int GUID_SIZE     = 1 + 1 + 1 + 4;
   public static final int CLASS_ID_SIZE = 1 + 1 + 1 + 1;
   public static final int HEADER_SIZE   = SIZE_SIZE + GUID_SIZE + CLASS_ID_SIZE;

   private final Cache[]           _caches     = new Cache[256];
   private final Dispatcher        _dispatcher = new Dispatcher( this );
   private final ByteBuffer        _header     = ByteBuffer.allocate( HEADER_SIZE );
   private final ByteBuffer        _payload    = ByteBuffer.allocate( 10*1024 );
   private final ByteBuffer        _frame      = ByteBuffer.allocate( 500*1024 );
   private final Map<ClassID,
      Supplier<Shareable>>         _factories  = new TreeMap<>();
   private final Map<Integer,
      BiConsumer<Integer,
         Map<String, Shareable>>>  _callbacks  = new HashMap<>();
   private final InetSocketAddress _target;
   private final DatagramChannel   _channel;
   private final byte              _platformId;
   private final byte              _execId;
   private /* */ int               _itemCount;
   private /* */ int               _callId;

   Repositories(
      InetAddress      group,
      NetworkInterface intrfc,
      int              port,
      byte             platformId,
      byte             execId      ) throws IOException
   {
      final ProtocolFamily family =
         ( group.getAddress().length > 4 )
            ? StandardProtocolFamily.INET6
            : StandardProtocolFamily.INET;
      _platformId = platformId;
      _execId     = execId;
      _target     = new InetSocketAddress( group, port );
      _channel    = DatagramChannel
         .open     ( family )
         .setOption( StandardSocketOptions.SO_REUSEADDR, true )
         .bind     ( new InetSocketAddress( port ))
         .setOption( StandardSocketOptions.IP_MULTICAST_IF  , intrfc )
//         .setOption( StandardSocketOptions.IP_MULTICAST_LOOP, false )
      ;
      _channel.join( group, intrfc );
   }

   Shareable newInstance( ClassID classId, ByteBuffer frame ) {
      final Supplier<Shareable> factory = _factories.get( classId );
      if( factory == null ) {
         return null;
      }
      final Shareable item = factory.get();
      item.unserialize( frame );
      return item;
   }

   @Override
   public void registerClass( ClassID id, Supplier<Shareable> factory ) {
      _factories.put( id, factory );
   }

   @Override
   public IRepository createRepository() {
      synchronized( _caches ) {
         final Cache cache = new Cache( this, _platformId, _execId );
         assert cache._cacheId > 0; // cache 0 doesn't exists, it's a flag for operation
         return _caches[cache._cacheId] = cache;
      }
   }

   @Override
   public IRepository getRepository( byte cacheId ) {
      synchronized( _caches ) {
         return _caches[cacheId];
      }
   }

   @Override
   public IDispatcher getDispatcher() {
      return _dispatcher;
   }

   private void initFrame( byte cacheId ) {
      _itemCount = 0;
      _frame.clear();
      _frame.put( SIGNATURE   );
      _frame.put( _platformId );
      _frame.put( _execId     );
      _frame.put( cacheId     ); // cache 0 doesn't exists, it's a flag for operation
      _frame.mark();
      _frame.putInt( _itemCount );
   }

   private void sendFrame() throws IOException {
      if( _itemCount > 0 ) {
         final int position = _frame.position();
         _frame.reset();
         _frame.putInt( _itemCount );
         _frame.position( position );
         _frame.flip();
         _channel.send( _frame, _target );
      }
      _frame.clear();
   }

   private void sendFrameIfNeeded( byte cacheId ) throws IOException {
      if( _frame.remaining() < ( _header.remaining() + _payload.remaining())) {
         sendFrame();
         initFrame( cacheId );
      }
   }

   private void pushCreateOrUpdateItem( byte cacheId, Shareable item ) throws IOException {
      _payload.clear();
      item.serialize( _payload );
      _payload.flip();
      final int size = _payload.remaining();
      _header.clear();
      _header.putInt( size );
      item._id   .serialize( _header );
      item._class.serialize( _header );
      _header.flip();
      sendFrameIfNeeded( cacheId );
      _frame.put( _header );
      _frame.put( _payload );
      ++_itemCount;
   }

   private void pushDeleteItem( byte cacheId, Shareable item ) throws IOException {
      _header.clear();
      _header.putInt( 0 );
      item._id.serialize( _header );
      _header.flip();
      sendFrameIfNeeded( cacheId );
      _frame.put( _header );
      ++_itemCount;
   }

   void publish(
      byte                     cacheId,
      Set<? extends Shareable> updated,
      Set<? extends Shareable> deleted ) throws IOException
   {
      synchronized( _frame ) {
         initFrame( cacheId );
         for( final Shareable item : updated ) {
            pushCreateOrUpdateItem( cacheId, item );
         }
         for( final Shareable item : deleted ) {
            pushDeleteItem( cacheId, item );
         }
         sendFrame();
      }
   }

   public void call( String intrfcName, String opName ) throws IOException {
      synchronized( _frame ) {
         initFrame((byte)0 );
         SerializerHelper.putString( intrfcName, _frame );
         SerializerHelper.putString( opName, _frame );
         _frame.flip();
         _channel.send( _frame, _target );
      }
   }

   public void call( String intrfcName, String opName, Map<String, Shareable> in ) throws IOException {
      synchronized( _frame ) {
         _frame.clear();
         _frame.put( SIGNATURE   );
         _frame.put( _platformId );
         _frame.put( _execId     );
         _frame.put( (byte)0     ); // cache 0 doesn't exists, it's a flag for operation
         _frame.putInt( in.size());
         SerializerHelper.putString( intrfcName, _frame );
         SerializerHelper.putString( opName, _frame );
         for( final Entry<String, Shareable> e : in.entrySet()) {
            SerializerHelper.putString( e.getKey(), _frame );
            final Shareable item = e.getValue();
            _payload.clear();
            item._class.serialize( _frame );
            item.serialize( _frame );
         }
         _frame.flip();
         _channel.send( _frame, _target );
      }
   }

   public int call(
      String                                      intrfcName,
      String                                      opName,
      Map<String, Shareable>                      in,
      BiConsumer<Integer, Map<String, Shareable>> callback ) throws IOException
   {
      synchronized( _frame ) {
         _frame.clear();
         _frame.put( SIGNATURE   );
         _frame.put( _platformId );
         _frame.put( _execId     );
         _frame.put( (byte)0     ); // cache 0 doesn't exists, it's a flag for operation
         _frame.putInt( in.size() + 1 );
         SerializerHelper.putString( intrfcName, _frame );
         SerializerHelper.putString( opName, _frame );
         for( final Entry<String, Shareable> e : in.entrySet()) {
            SerializerHelper.putString( e.getKey(), _frame );
            final Shareable item = e.getValue();
            item._class.serialize( _frame );
            item.serialize( _frame );
         }
         SerializerHelper.putString( "", _frame );
         _frame.putInt( _callId );
         _callbacks.put( _callId, callback );
         _frame.flip();
         _channel.send( _frame, _target );
         return _callId++;
      }
   }

   @Override
   public void run() {
      final byte[] signa = new byte[SIGNATURE.length];
      for( long atStart = 0;;) {
         try {
            final ByteBuffer frame = ByteBuffer.allocate( 64*1024 );
            if( atStart > 0 ) {
               Performance.record( "network", System.nanoTime() - atStart );
            }
            _channel.receive( frame );
            atStart = System.nanoTime();
            frame.flip();
//            Dump.dump( frame );
            frame.get( signa );
            if( Arrays.equals( signa, SIGNATURE )) {
               final byte platformId = frame.get();
               final byte execId     = frame.get();
               final byte cacheId    = frame.get();
               final int  count      = frame.getInt();
               final Map<String, Shareable> arguments = new HashMap<>();
               if( cacheId == 0 ) { // cache 0 doesn't exists, it's a flag for operation
                  final String intrfcName = SerializerHelper.getString( frame );
                  final String opName     = SerializerHelper.getString( frame );
                  int callId = -1;
                  for( int i = 0; i < count; ++i ) {
                     final String argName = SerializerHelper.getString( frame );
                     if( argName.isEmpty()) {
                        assert i == ( count - 1 ) :
                           "Only the last argument may be empty and for internal use only";
                        callId = _frame.getInt();
                     }
                     else {
                        final ClassID   classId = ClassID.unserialize( frame );
                        final Shareable item    = newInstance( classId, frame );
                        arguments.put( argName, item );
                     }
                  }
                  if( count == 0 ) {
                     _dispatcher.call( intrfcName, opName );
                  }
                  else if( callId < 0 ) {
                     _dispatcher.call( intrfcName, opName, arguments );
                  }
                  else {
                     final Map<String, Shareable> out = new HashMap<>();
                     _dispatcher.call( intrfcName, opName, arguments, out );
                     // TODO construire un message à partir de 'callId' et 'out'
                  }
               }
               else {
                  for( int i = 0; i < count; ++i ) {
                     final int size = frame.getInt();
                     if( size == 0 ) {
                        final GUID id = GUID.unserialize( frame );
                        for( final Cache cache: _caches ) {
                           cache.deleteFromNetwork( id );
                        }
                     }
                     else {
                        synchronized( _caches ) {
                           for( int j = 1; j < _caches.length; ++j ) {
                              final Cache cache = _caches[j];
                              if( cache == null ) {
                                 break;
                              }
                              if( ! cache.matches( platformId, execId, cacheId )) {
                                 cache.updateFromNetwork( frame.duplicate());
                              }
                           }
                        }
                        frame.position( frame.position() + GUID_SIZE + CLASS_ID_SIZE + size );
                     }
                  }
               }
               assert frame.remaining() == 0;
            }
            else {
               System.err.printf( "Garbage received, %d bytes discarded!\n", frame.limit());
            }
         }
         catch( final Throwable t ){
            t.printStackTrace();
         }
      }
   }
}
