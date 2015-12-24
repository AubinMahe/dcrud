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

import org.hpms.dbg.Performance;
import org.hpms.mw.distcrud.ClassID.Type;
import org.hpms.mw.distcrud.IRequired.CallMode;

final class NetworkReceiver extends Thread implements IProtocol {

   private final ParticipantImpl _participant;
   private final Dispatcher      _dispatcher;
   private final DatagramChannel _in;

   NetworkReceiver(
      ParticipantImpl   participant,
      InetSocketAddress source,
      NetworkInterface  intrfc ) throws IOException
   {
      _participant = participant;
      _dispatcher  = (Dispatcher)participant.getDispatcher();
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

   private void dataUpdate( ByteBuffer b ) {
      final int payloadSize = GUID_SIZE + CLASS_ID_SIZE + b.getInt();
      _participant.dataUpdate( b, payloadSize );
      b.position( b.position() + payloadSize );
   }

   private void dataDelete( ByteBuffer b ) {
      _participant.dataDelete( GUID.unserialize( b ));
   }

   private void operation( ByteBuffer b ) throws IOException {
      final Arguments args       = new Arguments();
      final int       queueNdx   = IRequired.DEFAULT_QUEUE;
      final CallMode  callMode   = IRequired.CallMode.ASYNCHRONOUS_DEFERRED;
      final String    intrfcName = SerializerHelper.getString( b );
      final String    opName     = SerializerHelper.getString( b );
      final int       callId     = b.getInt();
      final int       count      = b.get();
      for( int i = 0; i < count; ++i ) {
         final String  name    = SerializerHelper.getString( b );
         final ClassID classId = ClassID.unserialize( b );
         final Type    type    = classId.getType();
         switch( type ) {
         case NULL       : args.putNull( name );                                    break;
         case BYTE       : args.put( name, b.get());                                break;
         case BOOLEAN    : args.put( name, SerializerHelper.getBoolean( b ));       break;
         case SHORT      : args.put( name, b.getShort ());                          break;
         case INTEGER    : args.put( name, b.getInt   ());                          break;
         case LONG       : args.put( name, b.getLong  ());                          break;
         case FLOAT      : args.put( name, b.getFloat ());                          break;
         case DOUBLE     : args.put( name, b.getDouble());                          break;
         case STRING     : args.put( name, SerializerHelper.getString ( b ));       break;
         case CLASS_ID   : args.put( name, ClassID        .unserialize( b ));       break;
         case GUID       : args.put( name, GUID           .unserialize( b ));       break;
         case CALL_MODE  : args.setMode ( CallMode.values()[b.get()]);              break;
         case QUEUE_INDEX: args.setQueue( b.get());                                 break;
         case SHAREABLE  : args.put( name, _participant.newInstance( classId, b )); break;
         default:
            throw new IllegalStateException( "Unexpected type " + type + " for argument " + name );
         }
      }
      if( intrfcName.equals( ICRUD_INTERFACE_NAME )) {
         _dispatcher.executeCrud( opName, args );
      }
      else if( callId >= 0 ) {
         _dispatcher.execute( intrfcName, opName, callId, args, queueNdx, callMode );
      }
      else if( callId < 0 ) {
         _participant.callback( intrfcName, opName, args, -callId );
      }
   }

   @Override
   public void run() {
      final byte[]     signa = new byte[SIGNATURE.length];
      final ByteBuffer inBuf = ByteBuffer.allocate( 64*1024 );
      for( long atStart = 0;;) {
         try {
            if( atStart > 0 ) {
               Performance.record( "network", System.nanoTime() - atStart );
            }
            inBuf.clear();
            _in.receive( inBuf );
            atStart = System.nanoTime();
            inBuf.flip();
//            Dump.dump( inBuf );
            inBuf.get( signa );
            if( Arrays.equals( signa, SIGNATURE )) {
               final FrameType frameType = FrameType.values()[inBuf.get()];
               switch( frameType ) {
               case DATA_CREATE_OR_UPDATE: dataUpdate( inBuf ); break;
               case DATA_DELETE          : dataDelete( inBuf ); break;
               case OPERATION            : operation ( inBuf ); break;
               case NO_OP:
               default   :
                  throw new IllegalStateException();
               }
               assert inBuf.remaining() == 0;
            }
            else {
               System.err.printf( "Garbage received, %d bytes discarded!\n", inBuf.limit());
            }
         }
         catch( final Throwable t ){
            t.printStackTrace();
         }
      }
   }
}