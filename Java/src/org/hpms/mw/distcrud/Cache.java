package org.hpms.mw.distcrud;

import java.io.IOException;
import java.util.Collection;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.TreeMap;
import java.util.TreeSet;
import java.util.function.Function;
import java.util.function.Predicate;
import java.util.stream.Collectors;

final class Cache<T extends Shareable> implements IRepository<T> {

   private final Set<T>            _updated  = new HashSet<>();
   private final Set<T>            _deleted  = new HashSet<>();
   private final Set<T>            _toUpdate = new HashSet<>();
   private final Set<GUID>         _toDelete = new TreeSet<>();
   private final Map<GUID, T>      _local    = new TreeMap<>();
   private final int               _sourceId;
   private final boolean           _producer;
   private final Function<Integer,
      ? extends Shareable>         _factory;
   private final Repositories      _network;
   private /* */ int               _lastInstanceId;

   Cache(
      int                                    sourceId,
      boolean                                producer,
      Function<Integer, ? extends Shareable> factory,
      Repositories                           network )
   {
      _sourceId = sourceId;
      _producer = producer;
      _factory  = factory;
      _network  = network;
   }

   Collection<T> getContents() {
      return _local.values();
   }

   Shareable newInstance( int classId ) {
      return _factory.apply( classId );
   }

   @Override
   public void create( T item ) {
      if( item.getId() != null ) {
         throw new IllegalArgumentException( "Item already published!" );
      }
      if( ! _producer ) {
         throw new IllegalStateException( "Only owner can create!" );
      }
      item.setId( new GUID( _sourceId, ++_lastInstanceId ));
      synchronized( _local ) {
         _local.put( item.getId(), item );
      }
      synchronized( _updated ) {
         _updated.add( item );
      }
   }

   @Override
   public T read( GUID id ) {
      return _local.get( id );
   }

   @Override
   public Map<GUID, T> select( Predicate<T> query ) {
      return _local.values()
         .parallelStream()
         .filter( query )
         .collect( Collectors.toConcurrentMap( T::getId, Function.identity()));
   }

   @Override
   public void update( T item ) {
      if( item.getId() == null ) {
         throw new IllegalArgumentException( "Item must be created first!" );
      }
      if( ! _producer ) {
         throw new IllegalStateException( "Only owner can update!" );
      }
      if( ! _local.containsKey( item.getId())) {
         throw new IllegalArgumentException( "Repository doesn't contains item to update!" );
      }
      synchronized( _updated ) {
         _updated.add( item );
      }
   }

   @Override
   public void delete( T item ) {
      if( ! item.getId().matchClass( _sourceId )) {
         throw new IllegalArgumentException( "Repository mismatch!" );
      }
      if( ! _producer ) {
         throw new IllegalStateException( "Only owner can update!" );
      }
      synchronized( _local ) {
         _local.remove( item.getId());
      }
      synchronized( _deleted ) {
         _deleted.add( item );
      }
   }

   @Override
   public boolean isProducer() {
      return _producer;
   }

   @Override
   public boolean isConsumer() {
      return ! _producer;
   }

   @Override
   public void publish() throws IOException {
      synchronized( _updated ) {
         synchronized( _deleted ) {
            _network.publish( _updated, _deleted );
            _deleted.clear();
         }
         _updated.clear();
      }
   }

   @Override
   public void refresh() {
      synchronized( _local ) {
         synchronized( _toUpdate ) {
            for( final T item : _toUpdate ) {
               final T t = _local.get( item.getId());
               if( t == null ) {
                  _local.put( item.getId(), item );
               }
               else {
                  t.set( item );
               }
            }
            _toUpdate.clear();
         }
         synchronized( _toDelete ) {
            for( final GUID id : _toDelete ) {
               _local.remove( id );
            }
            _toDelete.clear();
         }
      }
   }

   @SuppressWarnings("unchecked")
   void updateFromNetwork( Shareable item ) {
      synchronized( _toUpdate ) {
         _toUpdate.add((T)item );
      }
   }

   void deleteFromNetwork( GUID id ) {
      synchronized( _toDelete ) {
         _toDelete.add( id );
      }
   }
}
