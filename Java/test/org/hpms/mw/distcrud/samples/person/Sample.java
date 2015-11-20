package org.hpms.mw.distcrud.samples.person;

import java.io.IOException;
import java.time.LocalDate;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;

import org.hpms.mw.distcrud.ICache;
import org.hpms.mw.distcrud.IDispatcher;
import org.hpms.mw.distcrud.IParticipant;
import org.hpms.mw.distcrud.IProvided;
import org.hpms.mw.distcrud.IRequired;
import org.hpms.mw.distcrud.IRequired.CallMode;
import org.hpms.mw.distcrud.Networks;
import org.hpms.mw.distcrud.Shareable;
import org.hpms.mw.distcrud.samples.Settings;

/**
 * Here is the main loop<ol>
 * <li>The consumer send an event to the producer which create a Person and publish it.
 * <li>The consumer wait 500 ms.
 * <li>The consumer refresh its cache and display it.
 * <li>The consumer wait 500 ms.
 * </ol>
 */
public class Sample implements Settings {

   private static final byte PLATFORM_ID = 1;
   private static final byte EXEC_ID     = 1;

   private static final Map<String, Object> _arguments = new HashMap<>();
   private static /* */ IRequired           _iPerson;

   private static boolean create( ICache repository, Map<String, Object> arguments ) {
      try {
         final Item item = (Item)arguments.get( "newPerson" );
         repository.create( item );
         System.err.printf( "producer|%s\n", item );
         repository.publish();
         return true;
      }
      catch( final Throwable t ) {
         t.printStackTrace();
      }
      return false;
   }

   private static void producer( IParticipant participant ) {
      try {
         final ICache repository = participant.createCache();
         final IDispatcher dispatcher = participant.getDispatcher();
         final IProvided   iPerson    = dispatcher.provide( "IPerson" );
         iPerson.addOperation( "create", ( in, out ) -> create( repository, in ));
         iPerson.addOperation( "exit"  , ( in, out ) -> { System.exit(0); return true; });
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

   public static void main( String[] args ) throws IOException {
      final IParticipant participant =
         Networks.join( MC_GROUP, MC_INTRFC, MC_PORT, PLATFORM_ID, EXEC_ID );
      participant.registerClass( Item.CLASS_ID, Item::new );
      new Thread(() -> consumer( participant )).start();
      new Thread(() -> producer( participant )).start();
      participant.run();
   }
}
