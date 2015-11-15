package org.hpms.mw.distcrud.samples.person;

import java.time.LocalDate;
import java.util.Set;

import org.hpms.mw.distcrud.IRepository;
import org.hpms.mw.distcrud.IRepositoryFactory;
import org.hpms.mw.distcrud.RepositoryFactoryBuilder;
import org.hpms.mw.distcrud.Shareable;
import org.hpms.mw.distcrud.samples.Settings;

public class Sample implements Settings {

   private static final byte PLATFORM_ID = 1;
   private static final byte EXEC_ID     = 1;

   private static void producer() {
      try {
         final IRepositoryFactory repositories =
            RepositoryFactoryBuilder.join( MC_GROUP, MC_INTRFC, MC_PORT, PLATFORM_ID, EXEC_ID );
         final IRepository repository = repositories.createRepository();
         for( int i = 0; i < 100; ++i ) {
            final Item item = new Item( "Aubin", "Mahé", LocalDate.parse( "1966-01-24" ), i );
            repository.create( item );
            System.err.printf( "producer|id: %s\n", item );
            repository.publish();
            Thread.sleep( 100 );
         }
      }
      catch( final Throwable t ) {
         t.printStackTrace();
      }
   }

   private static void consumer() {
      try {
         final IRepositoryFactory repositories =
            RepositoryFactoryBuilder.join( MC_GROUP, MC_INTRFC, MC_PORT, PLATFORM_ID, EXEC_ID );
         final IRepository repository = repositories.createRepository();
         repository.subscribe( Item.CLASS_ID, Item::new );
         for( int i = 0; i < 10; ++i ) {
            Thread.sleep( 1000 );
            repository.refresh();
            final Set<Shareable> items = repository.select( t -> true );
            for( final Shareable item : items ) {
               System.err.printf( "consumer|%s\n", item );
            }
         }
      }
      catch( final Throwable t ) {
         t.printStackTrace();
      }
   }

   public static void main( String[] args ) {
      new Thread( Sample::consumer ).start();
      producer();
      System.exit( 0 );
   }
}
