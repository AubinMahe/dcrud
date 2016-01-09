package org.hpms.mw.distcrud;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.DatagramChannel;
import java.util.Arrays;

import org.hpms.util.Performance;

abstract class DatagramNetworkReceiver extends AbstractNetworkReceiver {

   protected final DatagramChannel _in;

   DatagramNetworkReceiver( AbstractParticipant participant, DatagramChannel in ) throws IOException {
      super( participant );
      _in = in;
      setName( "Receiver-" + _in.getLocalAddress().toString());
      setDaemon( true );
      start();
   }

   @Override
   public final void run() {
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