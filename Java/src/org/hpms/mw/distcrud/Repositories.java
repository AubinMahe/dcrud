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
import java.util.LinkedList;
import java.util.List;
import java.util.Set;

final class Repositories extends Thread implements IRepositoryFactory {

   private static final byte[] SIGNATURE = { 'D','I','S','T','C','R','U','D'};

   public static final int SIZE_SIZE     = 4;
   public static final int GUID_SIZE  = 1 + 1 + 1 + 4;
   public static final int CLASS_ID_SIZE = 1 + 1 + 1 + 1;
   public static final int HEADER_SIZE   = SIZE_SIZE + GUID_SIZE + CLASS_ID_SIZE;

   private final List<Cache>       _caches  = new LinkedList<>();
   private final ByteBuffer        _header  = ByteBuffer.allocate( HEADER_SIZE );
   private final ByteBuffer        _payload = ByteBuffer.allocate( 10*1024 );
   private final ByteBuffer        _frame   = ByteBuffer.allocate( 500*1024 );
   private final InetSocketAddress _target;
   private final DatagramChannel   _channel;
   private final byte              _platformId;
   private final byte              _execId;
   private /* */ int               _itemCount;

   Repositories(
      InetAddress       group,
      NetworkInterface  intrfc,
      int               port,
      byte              platformId,
      byte              execId      ) throws IOException
   {
      super( Repositories.class.getSimpleName() + "-" + group + ":" + port );
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
      start();
   }

   @Override
   public IRepository createRepository() {
      synchronized( _caches ) {
         final Cache repo = new Cache( this, _platformId, _execId );
         _caches.add( repo );
         return repo;
      }
   }

   private void initFrame( byte cacheId ) {
      _itemCount = 0;
      _frame.clear();
      _frame.put( SIGNATURE   );
      _frame.put( _platformId );
      _frame.put( _execId     );
      _frame.put( cacheId     );
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
                        for( final Cache cache : _caches ) {
                           if( ! cache.matches( platformId, execId, cacheId )) {
                              cache.updateFromNetwork( frame.duplicate());
                           }
                        }
                     }
                     frame.position( frame.position() + GUID_SIZE + CLASS_ID_SIZE + size );
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
