package org.hpms.mw.distcrud;

import java.io.File;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.NetworkInterface;
import java.util.Enumeration;
import java.util.LinkedList;
import java.util.List;

import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;

public class Network {

   public static IParticipant join( File networkConfFile, String intrfcName, String name ) throws IOException, SAXException, ParserConfigurationException {
      if( networkConfFile == null ) {
         throw new IllegalArgumentException( "Network configuration file can't be null" );
      }
      if( intrfcName == null ) {
         throw new IllegalArgumentException( "Network interface name can't be null" );
      }
      if( name == null ) {
         throw new IllegalArgumentException( "Participant name can't be null" );
      }
      final NetworkInterface intrfc = NetworkInterface.getByName( intrfcName );
      if( intrfc == null ) {
         for( final Enumeration<NetworkInterface> e = NetworkInterface.getNetworkInterfaces();
            e.hasMoreElements(); )
         {
            final NetworkInterface ni = e.nextElement();
            if( ni.isUp()) {
               System.err.println( ni + " ==> virtual: " + ni.isVirtual());
            }
         }
         throw new IllegalArgumentException( intrfcName + " isn't a valid network interface!" );
      }
      if( ! intrfc.isUp()) {
         throw new IllegalStateException( intrfc + " must be up!" );
      }
      InetSocketAddress thisEndPoint = null;
      byte              id           = 0;
      final List<InetSocketAddress> publishers = new LinkedList<>();
      final Document                conf       =
         DocumentBuilderFactory.newInstance().newDocumentBuilder().parse( networkConfFile );
      final NodeList participants = conf.getElementsByTagName( "participant" );
      for( int i = 0, count = participants.getLength(); i < count; ++i ) {
         final Element           p        = (Element)participants.item( i );
         final String            n        = p.getAttribute( "name" );
         final String            address  = p.getAttribute( "address" );
         final int               port     = Integer.parseInt( p.getAttribute( "port" ));
         final InetSocketAddress endPoint = new InetSocketAddress( address, port );
         if( ! endPoint.getAddress().isMulticastAddress()) {
            throw new IllegalArgumentException(
               endPoint + " isn't a valid multicast address!" );
         }
         if( name.equals( n )) {
            thisEndPoint = endPoint;
            id           = Byte.parseByte( p.getAttribute( "id" ));
         }
         else {
            publishers.add( endPoint );
         }
      }
      if( thisEndPoint == null ) {
         throw new IllegalArgumentException(
            "Participant " + name + " not declared in network configuration file" );
      }
      final ParticipantImpl participant = new ParticipantImpl( id, thisEndPoint, intrfc );
      for( final InetSocketAddress endPoint : publishers ) {
         new NetworkReceiver( participant, endPoint, intrfc );
      }
      return participant;
   }
}
