package org.hpms.mw.distcrud.tooling;

import java.io.PrintStream;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Map.Entry;

import javax.xml.parsers.DocumentBuilderFactory;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

public final class XML2CfgConverter {

   public static void main( String[] args ) throws Exception {
      if( args.length != 2 ) {
         System.err.println(
            "Usage: " + XML2CfgConverter.class.getName() + " <xml file> <cfg file>" );
         System.exit( 1 );
      }
      final String xml = args[0];
      final String cfg = args[1];
      final Document conf = DocumentBuilderFactory.newInstance().newDocumentBuilder().parse( xml );
      final NodeList prtcpnts = conf.getElementsByTagName( "participant" );
      final Map<String, Element> participants = new LinkedHashMap<>();
      for( int i = 0, count = prtcpnts.getLength(); i < count; ++i ) {
         final Element p        = (Element)prtcpnts.item( i );
         final String  n        = p.getAttribute( "name" );
         participants.put( n, p );
      }
      try( final PrintStream cfgStream = new PrintStream( cfg )) {
         cfgStream.println( "#------- GENERATED FILE, DO NOT EDIT -------" );
         cfgStream.println( "#-------------------------------------------" );
         cfgStream.printf( "#%3s%18s%6s  %s\n", "id", "address", "port", "subscribe-to" );
         cfgStream.println( "#-------------------------------------------" );
         for( final Entry<String, Element> e : participants.entrySet()) {
            final Element  elt         = e.getValue();
            final String   id          = elt.getAttribute( "id" );
            final String   address     = elt.getAttribute( "address" );
            final String   port        = elt.getAttribute( "port" );
            final NodeList subscribers = elt.getElementsByTagName( "subscribe" );
            cfgStream.printf( " %3s%18s%6s  ", id, address, port );
            for( int i = 0, count = subscribers.getLength(); i < count; ++i ) {
               if( i > 0 ) {
                  cfgStream.print( ',' );
               }
               final Element xSubscriber  = (Element)subscribers.item(i);
               final String  subscriber   = xSubscriber.getAttribute( "to" );
               final Element xParticipant = participants.get( subscriber );
               assert xParticipant != null : subscriber + " not found!";
               final String  refId        = xParticipant.getAttribute( "id" );
               cfgStream.print( refId );
            }
            cfgStream.println();
         }
         cfgStream.println( "#-------------------------------------------" );
         cfgStream.println( "#------- GENERATED FILE, DO NOT EDIT -------" );
      }
      System.out.printf( "%s successfully converted to %s", xml, cfg );
   }
}
