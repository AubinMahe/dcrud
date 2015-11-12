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
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.function.Function;

final class Repositories extends Thread implements IRepositoryFactory {

   private static final byte[] SIGNATURE = { 'D','I','S','T','C','R','U','D'};

   private final Map<String,
      List<Cache<
         ? extends Shareable>>>    _repositories = new HashMap<>();
   private final ByteBuffer        _header       = ByteBuffer.allocate(   2*1024 );
   private final ByteBuffer        _payload      = ByteBuffer.allocate(  10*1024 );
   private final ByteBuffer        _frame        = ByteBuffer.allocate( 500*1024 );
   private final int               _id;
   private final DatagramChannel   _channel;
   private final InetSocketAddress _target;
   private /* */ int               _messageCount;

   Repositories( InetAddress group, NetworkInterface intrfc, int port, int id ) throws IOException {
      super( Repositories.class.getSimpleName() + "-" + group + ":" + port );
      _id = id;
      final ProtocolFamily family =
         ( group.getAddress().length > 4 )
            ? StandardProtocolFamily.INET6
            : StandardProtocolFamily.INET;
      _channel = DatagramChannel
         .open     ( family )
         .setOption( StandardSocketOptions.SO_REUSEADDR, true )
         .bind     ( new InetSocketAddress( port ))
         .setOption( StandardSocketOptions.IP_MULTICAST_IF, intrfc );
      _channel.join( group, intrfc );
      _target  = new InetSocketAddress( group, port );
      start();
   }

   @Override
   public <T extends Shareable>
   IRepository<T> createRepository( String topic, Function<GUID, ? extends Shareable> factory ) {
      final Cache<? extends Shareable> repo = new Cache<>( topic, factory, this );
      List<Cache<? extends Shareable>> repositories = _repositories.get( topic );
      if( repositories == null ) {
         _repositories.put( topic, repositories = new LinkedList<>());
      }
      repositories.add( repo );
      @SuppressWarnings("unchecked")
      final Cache<T> retVal = (Cache<T>)repo;
      return retVal;
   }

   private void initFrame() {
      _messageCount = 0;
      _frame.clear();
      _frame.put( SIGNATURE );
      _frame.putInt( _id );
      _frame.putInt( _messageCount );
   }

   private void sendFrame() throws IOException {
      if( _messageCount > 0 ) {
         final int r = _frame.position();
         _frame.position( SIGNATURE.length + 4 );
         _frame.putInt( _messageCount );
         _frame.position( r );
         _frame.flip();
         _channel.send( _frame, _target );
      }
      _frame.clear();
   }

   private void sendFrameIfNeeded() throws IOException {
      if( _frame.remaining() < ( _header.remaining() + _payload.remaining())) {
         sendFrame();
         initFrame();
      }
   }

   private void pushCreateOrUpdateItem( Shareable item ) throws IOException {
      _payload.clear();
      item.serialize( _payload );
      _payload.flip();
      final int size = _payload.remaining();
      _header.clear();
      _header.putInt( size );
      item.getId().serialize( _header );
      _header.flip();
      sendFrameIfNeeded();
      _frame.put( _header );
      _frame.put( _payload );
      ++_messageCount;
   }

   private void pushDeleteItem( Shareable item ) throws IOException {
      _header.clear();
      _header.putInt( 0 );
      item.getId().serialize( _header );
      _header.flip();
      sendFrameIfNeeded();
      _frame.put( _header );
      ++_messageCount;
   }

   void publish( Set<? extends Shareable> updated, Set<? extends Shareable> deleted ) throws IOException {
      synchronized( _frame ){
         initFrame();
         for( final Shareable item : updated ) {
            pushCreateOrUpdateItem( item );
         }
         for( final Shareable item : deleted ) {
            pushDeleteItem( item );
         }
         sendFrame();
      }
   }

   @Override
   public void run() {
      final ByteBuffer frame = ByteBuffer.allocate( 64*1024 );
      final byte[]     signa = new byte[SIGNATURE.length];
      for(;;) {
         try {
            frame.clear();
            _channel.receive( frame );
            frame.flip();
//            Dump.dump( frame );
            frame.get( signa );
            if( Arrays.equals( signa, SIGNATURE )) {
               final int publisher = frame.getInt();
               if( publisher != _id ) {
                  final int count = frame.getInt();
                  for( int i = 0; i < count; ++i ) {
                     final int  size = frame.getInt();
                     final GUID id   = GUID.unserialize( frame );
                     final List<Cache<? extends Shareable>> repositories =
                        _repositories.get( id._topic );
                     if( repositories != null ) {
                        Shareable item = null; // If none subscriber exists, it will stay null
                        for( final IRepository<? extends Shareable> repository : repositories ) {
                           final Cache<? extends Shareable> cache =
                              (Cache<? extends Shareable>)repository;
                           if( size == 0 ) {
                              cache.deleteFromNetwork( id );
                           }
                           else {
                              /**
                               * Only one instance, shared by all caches.
                               * TODO evaluate this optimization...
                               */
                              if( item == null ) {
                                 item = cache.newInstance( id, frame );
                              }
                              cache.updateFromNetwork( item );
                           }
                        }
                        if(( item == null ) && ( size > 0 )) {
                           // If none subscriber exists, we skip data
                           frame.position( frame.position() + size );
                        }
                     }
                     else {
                        System.err.printf(
                           "No subscriber for '%s', skipping %d bytes\n", id.toString(), size );
                        frame.position( frame.position() + size );
                     }
                  }
               }
               else {
//                  System.err.printf( "Own publication discarted, skipping %d bytes\n",
//                     frame.limit());
               }
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
