package org.hpms.mw.distcrud;

import java.io.IOException;
import java.nio.ByteBuffer;

import org.hpms.mw.distcrud.Arguments.CallMode;
import org.hpms.mw.distcrud.ClassID.Type;

abstract class AbstractNetworkReceiver extends Thread implements IProtocol {

   protected final AbstractParticipant _participant;
   protected final Dispatcher          _dispatcher;

   AbstractNetworkReceiver( AbstractParticipant participant ) {
      _participant = participant;
      _dispatcher  = (Dispatcher)participant.getDispatcher();
   }

   protected void dataUpdate( ByteBuffer b ) {
      final int payloadSize = GUID_SIZE + CLASS_ID_SIZE + b.getInt();
      _participant.dataUpdate( b, payloadSize );
      b.position( b.position() + payloadSize );
   }

   protected void dataDelete( ByteBuffer b ) {
      _participant.dataDelete( GUID.unserialize( b ));
   }

   protected void operation( ByteBuffer b ) throws IOException {
      final Arguments args       = new Arguments();
      final int       queueNdx   = Arguments.DEFAULT_QUEUE;
      final CallMode  callMode   = Arguments.CallMode.ASYNCHRONOUS_DEFERRED;
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
}