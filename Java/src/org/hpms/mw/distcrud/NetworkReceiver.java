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
import org.hpms.mw.distcrud.ClassID.PredefinedType;
import org.hpms.mw.distcrud.IRequired.CallMode;

final class NetworkReceiver extends Thread {

   private final ParticipantImpl _participant;
   private final ByteBuffer      _inBuf = ByteBuffer.allocate( 64*1024 );
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

   private void dataUpdate() {
      final int payloadSize =
         ParticipantImpl.GUID_SIZE + ParticipantImpl.CLASS_ID_SIZE + _inBuf.getInt();
      _participant.dataUpdate( _inBuf, payloadSize );
      _inBuf.position( _inBuf.position() + payloadSize );
   }

   private void dataDelete() {
      _participant.dataDelete( GUID.unserialize( _inBuf ));
   }

   private void operation() throws IOException {
      final Map<String, Object> args       = new HashMap<>();
      /* */ int                 queueNdx   = IRequired.DEFAULT_QUEUE;
      /* */ CallMode            callMode   = IRequired.CallMode.ASYNCHRONOUS_DEFERRED;
      final int                 count      = _inBuf.getInt();
      final String              intrfcName = SerializerHelper.getString( _inBuf );
      final String              opName     = SerializerHelper.getString( _inBuf );
      final int                 callId     = _inBuf.getInt();
      for( int i = 0; i < count; ++i ) {
         final String argName = SerializerHelper.getString( _inBuf );
         if( argName.equals("@queue")) {
            queueNdx = _inBuf.get();
            if( queueNdx < 0 ) {
               queueNdx += 256;
            }
         }
         else if( argName.equals("@mode")) {
            callMode = CallMode.values()[_inBuf.get()];
         }
         else {
            final ClassID classId = ClassID.unserialize( _inBuf );
            if( classId.isPredefined()) {
               final PredefinedType predef = classId.getPredefinedTypeID();
               switch( predef ) {
               case NullType   : args.put( argName, null               ); break;
               case ByteType   : args.put( argName, _inBuf.get       ()); break;
               case BooleanType: args.put( argName, SerializerHelper.getBoolean( _inBuf )); break;
               case ShortType  : args.put( argName, _inBuf.getShort  ()); break;
               case IntegerType: args.put( argName, _inBuf.getInt    ()); break;
               case LongType   : args.put( argName, _inBuf.getLong   ()); break;
               case FloatType  : args.put( argName, _inBuf.getFloat  ()); break;
               case DoubleType : args.put( argName, _inBuf.getDouble ()); break;
               case StringType : args.put( argName, SerializerHelper.getString ( _inBuf )); break;
               case ClassIDType: args.put( argName, ClassID        .unserialize( _inBuf )); break;
               case GUIDType   : args.put( argName, GUID           .unserialize( _inBuf )); break;
               default: throw new IllegalStateException( "Unexpected " + predef );
               }
            }
            else {
               final Shareable item = _participant.newInstance( classId, _inBuf );
               args.put( argName, item );
            }
         }
      }
      if( callId > 0 ) {
         final Map<String, Object> out = new HashMap<>();
         _participant.execute( intrfcName, opName, args, out, queueNdx, callMode );
         if( ! out.isEmpty()) {
            _participant.call( intrfcName, opName, out, -callId );
         }
      }
      else if( callId < 0 ) {
         _participant.callback( intrfcName, opName, args, -callId );
      }
   }

   @Override
   public void run() {
      final byte[] signa = new byte[ParticipantImpl.SIGNATURE.length];
      for( long atStart = 0;;) {
         try {
            if( atStart > 0 ) {
               Performance.record( "network", System.nanoTime() - atStart );
            }
            _inBuf.clear();
            _in.receive( _inBuf );
            atStart = System.nanoTime();
            _inBuf.flip();
//            Dump.dump( frame );
            _inBuf.get( signa );
            if( Arrays.equals( signa, ParticipantImpl.SIGNATURE )) {
               final FrameType frameType = FrameType.values()[_inBuf.get()];
               switch( frameType ) {
               case DATA_CREATE_OR_UPDATE: dataUpdate(); break;
               case DATA_DELETE          : dataDelete(); break;
               case OPERATION            : operation (); break;
               default: throw new IllegalStateException();
               }
               assert _inBuf.remaining() == 0;
            }
            else {
               System.err.printf( "Garbage received, %d bytes discarded!\n", _inBuf.limit());
            }
         }
         catch( final Throwable t ){
            t.printStackTrace();
         }
      }
   }
}