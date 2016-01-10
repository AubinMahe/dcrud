package org.hpms.mw.dcrud;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.NetworkInterface;

final class UDPParticipant extends AbstractParticipant implements IProtocol {

   public UDPParticipant( int id, InetSocketAddress addr ) {
      super( id );
      // TODO Auto-generated constructor stub
   }

   @Override
   public void listen( NetworkInterface via, IRegistry registry, boolean dumpReceivedBuffer ) throws IOException {
      listen( registry, dumpReceivedBuffer );
   }

   @Override
   public void listen( IRegistry registry, boolean dumpReceivedBuffer ) throws IOException {
      for( final InetSocketAddress other : registry.getParticipants()) {
         new UDPNetworkReceiver( this, other, dumpReceivedBuffer );
      }
   }

   @Override
   void pushCreateOrUpdateItem( Shareable item ) throws IOException {
      // TODO Auto-generated method stub
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
