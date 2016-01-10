package tests.person;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.NetworkInterface;
import java.util.Collection;

import org.hpms.mw.dcrud.Arguments;
import org.hpms.mw.dcrud.Arguments.CallMode;
import org.hpms.mw.dcrud.ICRUD;
import org.hpms.mw.dcrud.ICache;
import org.hpms.mw.dcrud.IDispatcher;
import org.hpms.mw.dcrud.IParticipant;
import org.hpms.mw.dcrud.IRegistry;
import org.hpms.mw.dcrud.Network;
import org.hpms.mw.dcrud.Shareable;

public class Subscriber extends Thread {

   private final IParticipant _participant;

   Subscriber( int id, InetSocketAddress addr, NetworkInterface via, IRegistry registry ) throws IOException {
      super( Subscriber.class.getName());
      _participant = Network.join( id, addr, via );
      _participant.listen( via, registry );
      start();
   }

   @Override
   public void run() {
      try {
         final ICache      cache      = _participant.createCache();
         final IDispatcher dispatcher = _participant.getDispatcher();
         final ICRUD       iPerson    = dispatcher.requireCRUD( Person.CLASS_ID );
         _participant.registerLocalFactory( Person.CLASS_ID, Person::new );
         cache       .subscribe      ( Person.CLASS_ID );
         for( int i = 0; i < 10; ++i ) {
            System.out.printf( "subscriber|dump cache begin\n" );
            Collection<Shareable> items = cache.values();
            for( final Shareable item : items ) {
               System.out.printf( "subscriber|object in cache: %s\n", item );
            }
            System.out.printf( "subscriber|dump cache end\n" );
            items = cache.select( t -> ((Person)t).getName().equals( "Le Nain" ));
            while( items.size() > 1 ) {
               final Shareable item = items.iterator().next();
               iPerson.delete( item );
               items  .remove( item );
               System.out.printf( "subscriber|delete person %s %s remotely\n",
                  ((Person)item).getForname(), ((Person)item).getName());
            }
            items = cache.select( t -> {
               final Person mahe = (Person)t;
               return mahe.getName().equals( "Mahé" ) && mahe.getForname().equals( "Aubin" );
            });
            while( items.size() > 2 ) {
               final Shareable item = items.iterator().next();
               iPerson.delete( item );
               items  .remove( item );
               System.out.printf( "subscriber|delete person %s %s remotely\n",
                  ((Person)item).getForname(), ((Person)item).getName());
            }
            if( items.size() > 1 ) {
               items.iterator().next();
               final Shareable item = items.iterator().next();
               final Arguments how  = new Arguments();
               how.put( "forname"  , "Eve" );
               how.put( "birthdate", "2008-02-28" );
               how.setQueue( Arguments.NON_URGENT_QUEUE );
               iPerson.update( item, how );
               System.out.printf( "subscriber|update person %s ==> 'Eve' remotely\n",
                  ((Person)item).getForname());
            }
            items = cache.select( t -> {
               final Person mahe = (Person)t;
               return mahe.getName().equals( "Mahé" ) && mahe.getForname().equals( "Eve" );
            });
            while( items.size() > 1 ) {
               final Shareable item = items.iterator().next();
               iPerson.delete( item );
               items  .remove( item );
               System.out.printf( "subscriber|delete person %s %s remotely\n",
                  ((Person)item).getForname(), ((Person)item).getName());
            }
            Thread.sleep( 2000 );
            final Arguments args = new Arguments();
            if(( i % 2 ) > 0 ) {
               args.put( "forname"  , "Aubin" );
               args.put( "name"     , "Mahé" );
               args.put( "birthdate", "1966-01-24" );
            }
            else {
               args.put( "forname"  , "Muriel" );
               args.put( "name"     , "Le Nain" );
               args.put( "birthdate", "1973-01-26" );
            }
            System.out.printf( "subscriber|create person %s %s remotely\n",
               args.get( "forname" ), args.get( "name" ));

            iPerson.create( args );
            Thread.sleep( 20 );
            dispatcher.handleRequests();
            cache.refresh();
         }
         final Arguments args = new Arguments();
         args.setQueue( Arguments.URGENT_QUEUE );
         args.setMode( CallMode.SYNCHRONOUS );
         System.out.printf( "subscriber|IMonitor.exit\n" );
         dispatcher.require( "IMonitor" ).call( "exit" );
         dispatcher.handleRequests();
         Thread.sleep( 500 );
      }
      catch( final Throwable t ) {
         t.printStackTrace();
      }
   }
}
