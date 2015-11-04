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
import java.util.Timer;
import java.util.TimerTask;
import java.util.function.Function;

final class Repositories extends Thread implements IRepositoryFactory {

   private static final byte[] SIGNATURE = { 'D','I','S','T','C','R','U','D'};
   private static final int FRM_HEADER_SIZE = SIGNATURE.length + 4;// SIGNATURE, Count
   private static final int MSG_HEADER_SIZE = 4 + 4 + 4 + 4; // Size, Source, Instance, Class

   private final Map<Integer,
      List<Cache<
         ? extends Shareable>>>    _repositories = new HashMap<>();
   private final ByteBuffer        _header       = ByteBuffer.allocate( MSG_HEADER_SIZE );
   private final ByteBuffer        _payload      = ByteBuffer.allocate(  10*1024 );
   private final ByteBuffer        _frame        = ByteBuffer.allocate( 500*1024 );
   private final TimerTask         _publisher    = new TimerTask() { @Override public void run() {
      publishAll(); }};
   private final DatagramChannel   _channel;
   private final InetSocketAddress _target;
   private /* */ int               _messageCount;

   Repositories( InetAddress group, NetworkInterface intrfc, int port ) throws IOException {
      super( Repositories.class.getSimpleName() + "-" + group + ":" + port);
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
      new Timer().schedule( _publisher, 1000L );
   }

   @Override
   public <T extends Shareable> IRepository<T> getRepository(
      int                                    sourceId,
      boolean                                producer,
      Function<Integer, ? extends Shareable> factory )
   {
      final Cache<? extends Shareable> repo = new Cache<>( sourceId, producer, factory, this );
      List<Cache<? extends Shareable>> repositories = _repositories.get( sourceId );
      if( repositories == null ) {
         _repositories.put( sourceId, repositories = new LinkedList<>());
      }
      repositories.add( repo );
      @SuppressWarnings("unchecked")
      final Cache<T> retVal = (Cache<T>)repo;
      return retVal;
   }

   void publishAll() {
      try {
         boolean initDone = false;
         synchronized( _frame ){
            for( final List<Cache<? extends Shareable>> repositories : _repositories.values()) {
               for( final Cache<? extends Shareable> repository : repositories ) {
                  for( final Shareable item : repository.getContents()) {
                     if( ! initDone ) {
                        initDone = true;
                        initFrame();
                     }
                     pushCreateOrUpdateItem( item );
                  }
               }
            }
            if( initDone ) {
               sendFrame();
            }
         }
      }
      catch( final IOException x ) {
         x.printStackTrace();
      }
   }

   void publish( Set<? extends Shareable> updated, Set<? extends Shareable> deleted ) throws IOException {
      boolean initDone = false;
      for( final Shareable item : updated ) {
         if( ! initDone ) {
            initDone = true;
            initFrame();
         }
         pushCreateOrUpdateItem( item );
      }
      for( final Shareable item : deleted ) {
         if( ! initDone ) {
            initDone = true;
            initFrame();
         }
         pushDeleteItem( item );
      }
      if( initDone ) {
         sendFrame();
      }
   }

   private void initFrame() {
      _messageCount = 0;
      _frame.clear();
      _frame.put( SIGNATURE );
      _frame.putInt( _messageCount );
   }

   private void pushCreateOrUpdateItem( Shareable item ) throws IOException {
      _payload.clear();
      item.serialize( _payload );
      _payload.flip();
      final int size = _payload.remaining();
      _header.clear();
      _header.putInt( size );
      _header.putInt( item.getId()._source );
      _header.putInt( item.getId()._instance );
      _header.putInt( item.getClassId());
      _header.flip();
      if( _frame.remaining() < MSG_HEADER_SIZE + size ) {
         sendFrame();
         initFrame();
      }
      _frame.put( _header );
      _frame.put( _payload );
      ++_messageCount;
   }

   private void pushDeleteItem( Shareable item ) throws IOException {
      _header.clear();
      _header.putInt( 0 );
      _header.putInt( item.getId()._source );
      _header.putInt( item.getId()._instance );
      _header.putInt( item.getClassId());
      _header.flip();
      if( _frame.remaining() < MSG_HEADER_SIZE ) {
         sendFrame();
         initFrame();
      }
      _frame.put( _header );
      ++_messageCount;
   }

   private void sendFrame() throws IOException {
      final int r = _frame.position();
      _frame.position( SIGNATURE.length );
      _frame.putInt( _messageCount );
      _frame.position( r );
      _frame.flip();
      _channel.send( _frame, _target );
   }

   @Override
   public void run() {
      final ByteBuffer frame = ByteBuffer.allocate( FRM_HEADER_SIZE + 64*1024 );
      final byte[]     signa = new byte[SIGNATURE.length];
      for(;;) {
         try {
            frame.clear();
            _channel.receive( frame );
            frame.flip();
//            Dump.dump( frame );
            frame.get( signa );
            if( Arrays.equals( signa, SIGNATURE )) {
               final int count = frame.getInt();
               for( int i = 0; i < count; ++i ) {
                  final int  size     = frame.getInt();
                  final int  sourceId = frame.getInt();
                  final GUID id       = new GUID( sourceId, frame.getInt());
                  final int  classId  = frame.getInt();
                  final List<Cache<
                     ? extends Shareable>> repositories = _repositories.get( sourceId );
                  if( repositories != null ) {
                     Shareable item = null; // S'il n'existe aucun consommateur, il reste null
                     for( final IRepository<? extends Shareable> repository : repositories ) {
                        final Cache<? extends Shareable> cache = (Cache<? extends Shareable>)repository;
                        if( size == 0 ) {
                           cache.deleteFromNetwork( id );
                        }
                        else if( repository.isConsumer()) {
                           if( item == null ) {
                              item = cache.newInstance( classId );
                              item.unserialize( frame );
                              item.setId( id );
                           }
                           cache.updateFromNetwork( item );
                        }
                     }
                     if( item == null ) {
                        // S'il n'existe aucun consommateur, on saute les données
                        frame.position( frame.position() + size );
                     }
                  }
                  else {
                     System.err.printf(
                        "No consumer for source %d, skipping %d bytes, id:%s, class:%d\n",
                        sourceId, size, id.toString(), classId );
                     frame.position( frame.position() + size );
                  }
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
