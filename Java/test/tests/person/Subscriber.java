package tests.person;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.NetworkInterface;
import java.util.Collection;

import org.hpms.mw.distcrud.Arguments;
import org.hpms.mw.distcrud.ICRUD;
import org.hpms.mw.distcrud.ICache;
import org.hpms.mw.distcrud.IDispatcher;
import org.hpms.mw.distcrud.IParticipant;
import org.hpms.mw.distcrud.IRequired;
import org.hpms.mw.distcrud.IRequired.CallMode;
import org.hpms.mw.distcrud.Network;
import org.hpms.mw.distcrud.Shareable;

public class Subscriber extends Thread {

   private final IParticipant _participant;

   Subscriber( byte id, InetSocketAddress addr, NetworkInterface via, InetSocketAddress...others ) throws IOException {
      super( Subscriber.class.getName());
      _participant = Network.join( id, addr, via );
      _participant.listen( via, others );
      start();
   }

   @Override
   public void run() {
      try {
         final ICache      cache      = _participant.createCache();
         final IDispatcher dispatcher = _participant.getDispatcher();
         final ICRUD       iPerson    = dispatcher.requireCRUD( Person.CLASS_ID );
         _participant.registerFactory( Person.CLASS_ID, Person::new );
         cache       .subscribe      ( Person.CLASS_ID );
         for( int i = 0; i < 10; ++i ) {
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
            iPerson.create( args );
            Thread.sleep( 500 );
            {
               cache.refresh();
               System.err.printf( "consumer|dump cache begin\n" );
               Collection<Shareable> items = cache.values();
               for( final Shareable item : items ) {
                  System.err.printf( "consumer|object in cache: %s\n", item );
               }
               System.err.printf( "consumer|dump cache end\n" );
               items = cache.select( t -> ((Person)t).getName().equals( "Le Nain" ));
               while( items.size() > 1 ) {
                  final Shareable item = items.iterator().next();
                  iPerson.delete( item );
                  items  .remove( item );
               }
               items = cache.select( t -> {
                  final Person mahe = (Person)t;
                  return mahe.getName().equals( "Mahé" ) && mahe.getForname().equals( "Aubin" );
               });
               while( items.size() > 2 ) {
                  final Shareable item = items.iterator().next();
                  iPerson.delete( item );
                  items  .remove( item );
               }
               if( items.size() > 1 ) {
                  items.iterator().next();
                  final Shareable item = items.iterator().next();
                  final Arguments how  = new Arguments();
                  how.put( "forname"  , "Eve" );
                  how.put( "birthdate", "2008-02-28" );
                  how.setQueue( IRequired.NON_URGENT_QUEUE );
                  iPerson.update( item, how );
               }
               items = cache.select( t -> {
                  final Person mahe = (Person)t;
                  return mahe.getName().equals( "Mahé" ) && mahe.getForname().equals( "Eve" );
               });
               while( items.size() > 1 ) {
                  final Shareable item = items.iterator().next();
                  iPerson.delete( item );
                  items  .remove( item );
               }
            }
            Thread.sleep( 2000 );
         }
         final Arguments args = new Arguments();
         args.setQueue( IRequired.URGENT_QUEUE );
         args.setMode( CallMode.SYNCHRONOUS );
         dispatcher.require( "IMonitor" ).call( "exit" );
      }
      catch( final Throwable t ) {
         t.printStackTrace();
      }
   }
}
