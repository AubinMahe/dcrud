package org.hpms.mw.dcrud;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.NetworkInterface;
import java.net.ProtocolFamily;
import java.net.StandardProtocolFamily;
import java.nio.channels.DatagramChannel;

final class UDPParticipant extends AbstractParticipant {

   private final InetSocketAddress _target;
   private final DatagramChannel   _out;

   public UDPParticipant( int id, InetSocketAddress addr ) throws IOException {
      super( id );
      final ProtocolFamily family =
         ( addr.getAddress().getAddress().length > 4 )
            ? StandardProtocolFamily.INET6
            : StandardProtocolFamily.INET;
      _target = addr;
      _out    = DatagramChannel.open( family );
      createCache();
      System.out.printf( "Sending via UDP to %s:%d\n", addr.getHostName(), addr.getPort());
   }

   @Override
   public void listen( NetworkInterface via, IRegistry registry ) throws IOException {
      listen( registry );
   }

   @Override
   public void listen( IRegistry registry ) throws IOException {
      for( final InetSocketAddress other : registry.getParticipants()) {
         new UDPNetworkReceiver( this, other );
      }
   }

   @Override
   void pushCreateOrUpdateItem( Shareable item ) throws IOException {
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

   @Override
   void pushDeleteItem( Shareable item ) throws IOException {
      // TODO Auto-generated method stub
   }

   @Override
   void call( String intrfcName, String opName, Arguments args, int callId ) throws IOException {
      // TODO Auto-generated method stub
   }
}
