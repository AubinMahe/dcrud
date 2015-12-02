package org.hpms.mw.distcrud.samples.person;

import java.io.File;
import java.io.IOException;
import java.time.LocalDate;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;

import javax.xml.parsers.ParserConfigurationException;

import org.hpms.mw.distcrud.ICache;
import org.hpms.mw.distcrud.IDispatcher;
import org.hpms.mw.distcrud.IParticipant;
import org.hpms.mw.distcrud.IProvided;
import org.hpms.mw.distcrud.IRequired;
import org.hpms.mw.distcrud.IRequired.CallMode;
import org.hpms.mw.distcrud.Network;
import org.hpms.mw.distcrud.Shareable;
import org.xml.sax.SAXException;

/**
 * Here is the main loop<ol>
 * <li>The consumer send an event to the producer which create a Person and publish it.
 * <li>The consumer wait 500 ms.
 * <li>The consumer refresh its cache and display it.
 * <li>The consumer wait 500 ms.
 * </ol>
 */
public class Sample {

   private static final Map<String, Object> _arguments = new HashMap<>();
   private static /* */ IRequired           _iPerson;

   private static void create( ICache repository, Map<String, Object> arguments ) {
      try {
         final Item item = (Item)arguments.get( "newPerson" );
         repository.create( item );
         System.err.printf( "producer|%s\n", item );
         repository.publish();
      }
      catch( final Throwable t ) {
         t.printStackTrace();
      }
   }

   private static void producer( IParticipant participant ) {
      try {
         final ICache repository = participant.createCache();
         final IDispatcher dispatcher = participant.getDispatcher();
         final IProvided   iPerson    = dispatcher.provide( "IPerson" );
         iPerson.addOperation( "create", ( in, out ) -> create( repository, in ));
         iPerson.addOperation( "exit"  , ( in, out ) -> System.exit(0));
         for( int i = 0; i < 100; ++i ) {
            Thread.sleep( 100 );
            dispatcher.handleRequests();
         }
      }
      catch( final Throwable t ) {
         t.printStackTrace();
      }
   }

   private static void createPerson( Item item ) throws IOException {
      _arguments.clear();
      _arguments.put( "newPerson", item );
      _iPerson.call( "create", _arguments, null );
   }

   private static void consumer( IParticipant participant ) {
      try {
         final ICache repository = participant.createCache();
         final IDispatcher dispatcher = participant.getDispatcher();
         _iPerson = dispatcher.require( "IPerson" );
         repository.subscribe( Item.CLASS_ID );
         for( int i = 0; i < 10; ++i ) {
            Thread.sleep( 500 );
            createPerson( new Item( "Aubin", "Mahé", LocalDate.parse( "1966-01-24" ), i ));
            Thread.sleep( 500 );
            {
               repository.refresh();
               final Set<Shareable> items = repository.select( t -> true );
               for( final Shareable item : items ) {
                  System.err.printf( "consumer|%s\n", item );
               }
            }
         }
         _arguments.clear();
         _arguments.put("@queue", IRequired.URGENT_QUEUE );
         _arguments.put("@mode" , CallMode.SYNCHRONOUS );
         _iPerson.call( "exit", _arguments, null );
      }
      catch( final Throwable t ) {
         t.printStackTrace();
      }
   }

   public static void main( String[] args ) throws IOException, SAXException, ParserConfigurationException {
      final IParticipant participant =
         Network.join( new File( "network.xml"), "eth0", "Shapes-1" );
      participant.registerClass( Item.CLASS_ID, Item::new );
      new Thread(() -> consumer( participant )).start();
      new Thread(() -> producer( participant )).start();
   }
}
