package org.hpms.mw.channel;

import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.nio.ByteBuffer;
import java.nio.channels.DatagramChannel;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Map;

public class UDPChannel implements Runnable {

   private final Map<Short,
      IMessageHandler>           _handlers = new HashMap<>();
   private final DatagramChannel _channel  = DatagramChannel.open();
   private final CoDec           _decoder  = new CoDec();
   private /* */ ILogger         _logger;

   public UDPChannel( String intrfc, int local, String targetHost, int remote ) throws IOException {
      _channel.bind   ( new InetSocketAddress( InetAddress.getByName( intrfc     ), local  ));
      _channel.connect( new InetSocketAddress( InetAddress.getByName( targetHost ), remote ));
      final Thread t = new Thread( this );
      t.setDaemon( true );
      t.setName( getClass().getName());
      t.start();
   }

   public void addLogger( ILogger logger ) {
      _logger = logger;
   }

   public void addHandler( short msgId, IMessageHandler handler ) {
      _handlers.put( msgId, handler );
   }

   public void send( CoDec encoder ) throws IOException {
      final ByteBuffer buffer = encoder.getBuffer();
      if( buffer.position() > 0 ) {
         buffer.flip();
      }
      _channel.write( buffer );
   }

   @Override
   public void run() {
      try {
         final String name = Thread.currentThread().getName();
         if( _logger != null ) {
            _logger.printf( "'%s' running\n", name );
         }
         while( _channel.isOpen()) {
            final short           msgId   = _decoder.receive( _channel );
            final IMessageHandler handler = _handlers.get( msgId );
            if( _logger != null ) {
               _logger.printf( "'%s': message '%d' received, dump follows\n", name, msgId );
               _logger.printf( "%s", _decoder.dump());
            }
            if( handler == null ) {
               if( _logger != null ) {
                  _logger.printf( "'%s': No handler declared for message '%d'\n", name, msgId );
               }
            }
            else {
               try {
                  handler.decode( msgId, _decoder );
               }
               catch( final Throwable t ) {
                  t.printStackTrace();
               }
            }
         }
      }
      catch( final IOException e ) {
         e.printStackTrace();
      }
   }

   public static void showAvailableInterfaces() throws SocketException {
      final Enumeration<NetworkInterface> e1 = NetworkInterface.getNetworkInterfaces();
      while( e1.hasMoreElements()) {
         final NetworkInterface ni = e1.nextElement();
         if( ni.isUp() && !ni.isLoopback()) {
            System.err.println( ni );
            final Enumeration<InetAddress> e2 = ni.getInetAddresses();
            while( e2.hasMoreElements()) {
               final InetAddress addr = e2.nextElement();
               System.err.printf( "\t%s\n", addr );
            }
         }
      }
   }
}
