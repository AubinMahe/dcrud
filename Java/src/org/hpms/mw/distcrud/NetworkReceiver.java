package org.hpms.mw.distcrud;

import java.io.IOException;
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

import org.hpms.dbg.Performance;
import org.hpms.mw.distcrud.IRequired.CallMode;

final class NetworkReceiver extends Thread {

   private final ParticipantImpl _participant;
   private final DatagramChannel _in;

   NetworkReceiver(
      ParticipantImpl   participant,
      InetSocketAddress source,
      NetworkInterface  intrfc ) throws IOException
   {
      _participant = participant;
      final ProtocolFamily family =
         ( source.getAddress().getAddress().length > 4 )
            ? StandardProtocolFamily.INET6
            : StandardProtocolFamily.INET;
      _in = DatagramChannel
         .open     ( family )
         .setOption( StandardSocketOptions.SO_REUSEADDR, true )
         .bind     ( source )
         .setOption( StandardSocketOptions.IP_MULTICAST_IF  , intrfc )
      ;
      _in.join( source.getAddress(), intrfc );
      setName( "Receiver-" + source.toString());
      setDaemon( true );
      System.out.printf( "receiving from %s, bound to %s\n", source, intrfc );
      start();
   }

   private void dataDelete( ByteBuffer frame ) {
      final GUID id = GUID.unserialize( frame );
      synchronized( _participant._caches ) {
         for( final Cache cache : _participant._caches ) {
            if( cache == null ) {
               break;
            }
            cache.deleteFromNetwork( id );
         }
      }
   }

   private void dataUpdate( ByteBuffer frame ) {
      final int size        = frame.getInt();
      final int payloadSize = ParticipantImpl.GUID_SIZE + ParticipantImpl.CLASS_ID_SIZE + size;
      synchronized( _participant._caches ) {
         for( final Cache cache : _participant._caches ) {
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
      frame.position( frame.position() + payloadSize );
   }

   private void operation( ByteBuffer frame ) throws IOException {
      final Map<String, Object> arguments  = new HashMap<>();
      /* */ int                 queueNdx   = IRequired.DEFAULT_QUEUE;
      /* */ CallMode            callMode   = IRequired.CallMode.ASYNCHRONOUS_DEFERRED;
      final int                 count      = frame.getInt();
      final String              intrfcName = SerializerHelper.getString( frame );
      final String              opName     = SerializerHelper.getString( frame );
      final int                 callId     = frame.getInt();
      for( int i = 0; i < count; ++i ) {
         final String argName = SerializerHelper.getString( frame );
         if( argName.equals("@queue")) {
            queueNdx = frame.get();
            if( queueNdx < 0 ) {
               queueNdx += 256;
            }
         }
         else if( argName.equals("@mode")) {
            callMode = CallMode.values()[frame.get()];
         }
         else {
            final ClassID   classId = ClassID.unserialize( frame );
            final Shareable item    = _participant.newInstance( classId, frame );
            arguments.put( argName, item );
         }
      }
      if( callId > 0 ) {
         final Map<String, Object> out = new HashMap<>();
         _participant._dispatcher.execute( intrfcName, opName, arguments, out, queueNdx, callMode );
         if( ! out.isEmpty()) {
            _participant.call( intrfcName, opName, out, -callId );
         }
      }
      else if( callId < 0 ) {
         final ICallback callback = _participant._callbacks.get( -callId );
         if( callback == null ) {
            System.err.printf( "Unknown Callback received: %s.%s, id: %d\n",
               intrfcName, opName, -callId );
         }
         else {
            callback.callback( intrfcName, opName, arguments );
         }
      }
   }

   @Override
   public void run() {
      final byte[]     signa = new byte[ParticipantImpl.SIGNATURE.length];
      final ByteBuffer frame = ByteBuffer.allocate( 64*1024 );
      for( long atStart = 0;;) {
         try {
            if( atStart > 0 ) {
               Performance.record( "network", System.nanoTime() - atStart );
            }
            frame.clear();
            _in.receive( frame );
            atStart = System.nanoTime();
            frame.flip();
//            Dump.dump( frame );
            frame.get( signa );
            if( Arrays.equals( signa, ParticipantImpl.SIGNATURE )) {
               final FrameType frameType = FrameType.values()[frame.get()];
               switch( frameType ) {
               case DATA_CREATE_OR_UPDATE: dataUpdate( frame ); break;
               case DATA_DELETE          : dataDelete( frame ); break;
               case OPERATION            : operation ( frame ); break;
               default: throw new IllegalStateException();
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