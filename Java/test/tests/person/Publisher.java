package tests.person;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.NetworkInterface;
import java.time.LocalDate;

import org.hpms.mw.distcrud.Arguments;
import org.hpms.mw.distcrud.ICRUD;
import org.hpms.mw.distcrud.ICache;
import org.hpms.mw.distcrud.IDispatcher;
import org.hpms.mw.distcrud.IParticipant;
import org.hpms.mw.distcrud.IProvided;
import org.hpms.mw.distcrud.Network;
import org.hpms.mw.distcrud.Shareable;

public class Publisher extends Thread {

   static final class PersonPublisher implements ICRUD {

      private final ICache _cache;

      PersonPublisher( ICache cache ) {
         _cache = cache;
      }

      @Override
      public void create( Arguments how ) throws IOException {
         final String forname   = how.get( "forname" );
         final String name      = how.get( "name" );
         final String birthdate = how.get( "birthdate" );
         final Person person    = new Person( forname, name, LocalDate.parse( birthdate ));
         _cache.create( person );
         _cache.publish();
         System.err.printf( "%s.create|%s\n", getClass().getName(), person );
      }

      @Override
      public void update( Shareable what, Arguments how ) throws IOException {
         final Person person    = (Person)what;
         final String forname   = how.get( "forname" );
         final String birthdate = how.get( "birthdate" );
         person.update( forname, null, birthdate );
         _cache.update( person );
         _cache.publish();
         System.err.printf( "%s.update|%s\n", getClass().getName(), person );
      }

      @Override
      public void delete( Shareable what ) throws IOException {
         final Person person = (Person)what;
         _cache.delete( person );
         _cache.publish();
         System.err.printf( "%s.delete|%s\n", getClass().getName(), person );
      }
   }

   private final IParticipant _participant;

   Publisher( byte id, InetSocketAddress addr, NetworkInterface via, InetSocketAddress...others ) throws IOException {
      super( Publisher.class.getName());
      _participant = Network.join( id, addr, via );
      _participant.listen( via, others );
      start();
   }

   @Override
   public void run() {
      final ICache          cache      = _participant.getDefaultCache();
      final IDispatcher     dispatcher = _participant.getDispatcher();
      final PersonPublisher publisher  = new PersonPublisher( cache );
      final IProvided       iPerson    = dispatcher.provide( "IMonitor" );
      iPerson.addOperation( "exit", args -> { System.exit(0); return null; });
      _participant.registerFactory  ( Person.CLASS_ID, Person::new );
      _participant.registerPublisher( Person.CLASS_ID, publisher );
      for(;;) {
         try {
            Thread.sleep( 100 );
         }
         catch( final InterruptedException e ) {
            e.printStackTrace();
         }
         dispatcher.handleRequests();
      }
   }
}
