package tests.person;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.NetworkInterface;
import java.time.LocalDate;

import org.hpms.mw.dcrud.Arguments;
import org.hpms.mw.dcrud.ICRUD;
import org.hpms.mw.dcrud.ICache;
import org.hpms.mw.dcrud.IDispatcher;
import org.hpms.mw.dcrud.IParticipant;
import org.hpms.mw.dcrud.IRegistry;
import org.hpms.mw.dcrud.Network;
import org.hpms.mw.dcrud.Shareable;

public class Publisher extends Thread {

   static final class PersonPublisher implements ICRUD {

      private final ICache _cache;

      PersonPublisher( ICache cache ) {
         _cache = cache;
      }

      @Override
      public void create( Arguments how ) throws IOException {
         try {
            final String forname   = how.get( "forname" );
            final String name      = how.get( "name" );
            final String birthdate = how.get( "birthdate" );
            final Person person    = new Person( forname, name, LocalDate.parse( birthdate ));
            _cache.create( person );
            _cache.publish();
            System.err.printf( "%s.create|%s\n", getClass().getName(), person );
         }
         catch( final Throwable t ) {
            t.printStackTrace();
            try {
               System.err.println( how.toString());
            }
            catch( final Throwable tt ){
               tt.printStackTrace();
            }
            try {
               System.err.println( "Press <enter>" );
               System.in.read();
            }
            catch( final Throwable tt ){
               tt.printStackTrace();
            }
            System.exit(0);
         }
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

   Publisher( int id, InetSocketAddress addr, NetworkInterface via, IRegistry registry ) throws IOException {
      super( Publisher.class.getName());
      _participant = Network.join( id, addr, via );
      _participant.listen( via, registry );
      start();
   }

   @Override
   public void run() {
      System.err.println( getClass().getName() + ".run|entry" );
      final ICache          cache      = _participant.getDefaultCache();
      final IDispatcher     dispatcher = _participant.getDispatcher();
      final PersonPublisher publisher  = new PersonPublisher( cache );
      dispatcher
         .provide( "IMonitor" )
            .addOperation( "exit", args -> { System.exit(0); return null; });
      _participant.registerLocalFactory ( Person.CLASS_ID, Person::new );
      _participant.registerRemoteFactory( Person.CLASS_ID, publisher );
      System.err.println( getClass().getName() + ".run|enter infinite loop" );
      for(;;) {
         try {
            Thread.sleep( 100 );
            dispatcher.handleRequests();
         }
         catch( final Throwable t ) {
            t.printStackTrace();
            try {
               System.err.println( "Press <enter>" );
               System.in.read();
            }
            catch( final IOException e ){
               e.printStackTrace();
            }
         }
      }
   }
}
