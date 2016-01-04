package org.hpms.mw.distcrud;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.NetworkInterface;

final class UDPParticipant extends AbstractParticipant implements IProtocol {

   public UDPParticipant( int id, InetSocketAddress addr ) {
      super( id );
      // TODO Auto-generated constructor stub
   }

   @Override
   public void listen( NetworkInterface via, InetSocketAddress... others ) throws IOException {
      listen( others );
   }

   @Override
   public void listen( InetSocketAddress... others ) throws IOException {
      for( final InetSocketAddress other : others ) {
         new UDPNetworkReceiver( this, other );
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
