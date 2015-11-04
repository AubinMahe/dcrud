package org.hpms.mw.distcrud.sample1;

import java.time.LocalDate;
import java.util.Map;

import org.hpms.mw.distcrud.GUID;
import org.hpms.mw.distcrud.IRepository;
import org.hpms.mw.distcrud.IRepositoryFactory;
import org.hpms.mw.distcrud.RepositoryFactoryBuilder;

public class Sample {

   private static final String MC_GROUP   = "224.0.0.1";
   private static final int    MC_PORT    = 2416;
   private static final int    ITEM_CLASS = 123456;

   private static void producer() {
      try {
         final IRepositoryFactory repositories =
            RepositoryFactoryBuilder.join( MC_GROUP, "eth4", MC_PORT );
         final IRepository<Item>  repository   =
            repositories.getRepository( ITEM_CLASS, true, classId -> new Item());
         for( int i = 0; ; ++i ) {
            final Item item = new Item( "Aubin", "Mahé", LocalDate.parse( "1966-01-24" ), i );
            repository.create( item );
            System.err.printf( "producer|id: %s\n", item.getId());
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
            RepositoryFactoryBuilder.join( MC_GROUP, "eth4", MC_PORT );
         final IRepository<Item> repository   =
            repositories.getRepository( ITEM_CLASS, false, classId -> new Item());
         for(;;) {
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

   public static void main( String[] args ) {
      new Thread( Sample::producer ).start();
      new Thread( Sample::consumer ).start();
   }
}
