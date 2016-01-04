package org.hpms.mw.distcrud;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.NetworkInterface;
import java.net.ProtocolFamily;
import java.net.StandardProtocolFamily;
import java.net.StandardSocketOptions;
import java.nio.ByteBuffer;
import java.nio.channels.DatagramChannel;

final class MulticastParticipant extends AbstractParticipant implements IProtocol {

   private final ByteBuffer        _header  = ByteBuffer.allocate( HEADER_SIZE );
   private final ByteBuffer        _payload = ByteBuffer.allocate( PAYLOAD_SIZE );
   private final ByteBuffer        _message = ByteBuffer.allocate( 64*1024 );
   private final InetSocketAddress _target;
   private final DatagramChannel   _out;

   MulticastParticipant( int publisherId, InetSocketAddress group, NetworkInterface intrfc ) throws IOException {
      super( publisherId );
      final ProtocolFamily family =
         ( group.getAddress().getAddress().length > 4 )
            ? StandardProtocolFamily.INET6
            : StandardProtocolFamily.INET;
      _target      = group;
      _out         = DatagramChannel
         .open     ( family )
//         .setOption( StandardSocketOptions.SO_REUSEADDR, true )
//         .bind     ( group )
         .setOption( StandardSocketOptions.IP_MULTICAST_IF, intrfc )
      ;
      createCache();
      System.out.printf( "Sending to multicast address %s via interface %s\n", group, intrfc );
   }

   @Override
   public void listen( NetworkInterface via, InetSocketAddress...others ) throws IOException {
      for( final InetSocketAddress other : others ) {
         new MulticastNetworkReceiver( this, other, via );
      }
   }

   @Override
   public void listen( InetSocketAddress... others ) throws IOException {
      throw new IllegalArgumentException( "Multicast participant require an interface argument" );
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
      synchronized( _out ){
         _header.clear();
         _header.put( SIGNATURE );
         _header.put((byte)FrameType.DATA_DELETE.ordinal());
         item._id.serialize( _header );
         _header.flip();
         _out.send( _header, _target );
      }
   }

   @Override
   void call( String intrfcName, String opName, Arguments args, int callId ) throws IOException {
      synchronized( _out ) {
         final byte count = ( args == null ) ? 0 : (byte)args.getCount();
         _message.clear();
         _message        .put      ( SIGNATURE );
         _message        .put      ((byte)FrameType.OPERATION.ordinal());
         SerializerHelper.putString( intrfcName, _message );
         SerializerHelper.putString( opName    , _message );
         _message        .putInt   ( callId );
         _message        .put      ( count );
         if( args != null ) {
            args.serialize( _message );
         }
         _message.flip();
         _out.send( _message, _target );
      }
   }
}
