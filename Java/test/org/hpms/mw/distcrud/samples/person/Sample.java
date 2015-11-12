package org.hpms.mw.distcrud.samples.person;

import java.io.IOException;
import java.time.LocalDate;
import java.util.Map;

import org.hpms.mw.distcrud.GUID;
import org.hpms.mw.distcrud.IRepository;
import org.hpms.mw.distcrud.IRepositoryFactory;
import org.hpms.mw.distcrud.RepositoryFactoryBuilder;
import org.hpms.mw.distcrud.samples.Settings;

public class Sample implements Settings {

   private static void producer( IRepository<Item> repository ) {
      try {
         for( int i = 0; i < 100; ++i ) {
            final Item item = new Item( "Aubin", "Mahé", LocalDate.parse( "1966-01-24" ), i );
            repository.create( item, Item.CLASS );
            System.err.printf( "producer|id: %s\n", item.getId());
            repository.publish();
            Thread.sleep( 100 );
         }
      }
      catch( final Throwable t ) {
         t.printStackTrace();
      }
   }

   private static void consumer( IRepository<Item> repository ) {
      try {
         for( int i = 0; i < 10; ++i ) {
            Thread.sleep( 1000 );
            repository.refresh();
            final Map<GUID, Item> items = repository.select( t -> true );
            for( final Item item : items.values()) {
               System.err.printf( "consumer|id: %s\n", item.getId());
            }
         }
      }
      catch( final Throwable t ) {
         t.printStackTrace();
      }
   }

   public static void main( String[] args ) throws IOException {
      final IRepositoryFactory repositories =
         RepositoryFactoryBuilder.join( MC_GROUP, MC_INTRFC, MC_PORT, 1 );
      final IRepository<Item>  repository   =
         repositories.createRepository( Item.TOPIC, classId -> new Item());
      new Thread(() -> consumer( repository )).start();
      producer( repository );
      System.exit( 0 );
   }
}
