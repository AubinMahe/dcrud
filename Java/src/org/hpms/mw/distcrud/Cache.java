package org.hpms.mw.distcrud;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.Collection;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.TreeMap;
import java.util.TreeSet;
import java.util.function.Predicate;
import java.util.stream.Collectors;

import org.hpms.dbg.Performance;

final class Cache implements ICache {

   private static byte _NextCacheId = 1; // cache 0 doesn't exists, it's a flag for operation

   private final Set<ClassID>         _classes        = new HashSet<>();
   private final Set<Shareable>       _updated        = new HashSet<>();
   private final Set<Shareable>       _deleted        = new HashSet<>();
   private final Set<ByteBuffer>      _toUpdate       = new HashSet<>();
   private final Set<GUID>            _toDelete       = new TreeSet<>();
   private final Map<GUID, Shareable> _local          = new TreeMap<>();
   private /* */ int                  _nextInstance   = 1;
   private /* */ boolean              _ownershipCheck = false;
   private final Network              _network;
   private final byte                 _platformId;
   private final byte                 _execId;
   /*   */ final byte                 _cacheId;

   Cache( Network network, byte platformId, byte execId ) {
      _network    = network;
      _platformId = platformId;
      _execId     = execId;
      _cacheId    = _NextCacheId++;
   }

   boolean matches( byte platformId, byte execId, byte cacheId ) {
      return( platformId == _platformId )
         && ( execId     == _execId     )
         && ( cacheId    == _cacheId    );
   }

   @Override
   public boolean owns( GUID id ) {
      return matches( id._platform, id._exec, id._cache );
   }

   @Override
   public void setOwnership( boolean enabled ) {
      _ownershipCheck = enabled;
   }

   @Override
   public Status create( Shareable item ) {
      if( item._id.isShared()) {
         return Status.ALREADY_CREATED;
      }
      item._id._platform = _platformId;
      item._id._exec     = _execId;
      item._id._cache    = _cacheId;
      item._id._instance = _nextInstance++;
      synchronized( _local ) {
         _local.put( item._id, item );
      }
      synchronized( _updated ) {
         _updated.add( item );
      }
      return Status.NO_ERROR;
   }

   @SuppressWarnings("unchecked")
   @Override
   public <T extends Shareable> T read( GUID id ) {
      return (T)_local.get( id );
   }

   @Override
   public Status update( Shareable item ) {
      if( _ownershipCheck && ! owns( item._id )) {
         return Status.NOT_OWNER;
      }
      if( item._id._instance == 0 ) {
         return Status.NOT_CREATED;
      }
      if( ! _local.containsKey( item._id )) {
         return Status.NOT_IN_THIS_CACHE;
      }
      synchronized( _updated ) {
         _updated.add( item );
      }
      return Status.NO_ERROR;
   }

   @Override
   public Status delete( Shareable item ) {
      synchronized( _local ) {
         if( _ownershipCheck && ! owns( item._id )) {
            return Status.NOT_OWNER;
         }
         if( _local.remove( item._id ) == null ) {
            return Status.NOT_IN_THIS_CACHE;
         }
         synchronized( _deleted ) {
            _deleted.add( item );
         }
      }
      return Status.NO_ERROR;
   }

   @Override
   public Collection<Shareable> values() {
      return _local.values();
   }

   @Override
   public Set<Shareable> select( Predicate<Shareable> query ) {
      return _local.values()
         .parallelStream()
         .filter( query )
         .collect( Collectors.toSet());
   }

   @Override
   public void publish() throws IOException {
      final long atStart = System.nanoTime();
      synchronized( _updated ) {
         synchronized( _deleted ) {
            _network.publish( _cacheId, _updated, _deleted );
            _deleted.clear();
         }
         _updated.clear();
      }
      Performance.record( "publish", System.nanoTime() - atStart );
   }

   @Override
   public void subscribe( ClassID id ) {
      _classes.add( id );
   }

   @Override
   public void refresh() {
      final long atStart = System.nanoTime();
      synchronized( _local ) {
         synchronized( _toUpdate ) {
            for( final ByteBuffer update : _toUpdate ) {
               final GUID      id      = GUID.unserialize( update );
               final ClassID   classId = ClassID.unserialize( update );
               final Shareable t       = _local.get( id );
               if( t == null ) {
                  final Shareable item = _network.newInstance( classId, update );
                  if( item != null ) {
                     item._id.set( id );
                     _local.put( id, item );
                  }
                  else {
                     System.err.printf( "Unknown %s of %s\n", classId, id );
                  }
               }
               else if( ! _ownershipCheck || ! owns( id )) {
                  t.unserialize( update );
               }
            }
            _toUpdate.clear();
         }
         synchronized( _toDelete ) {
            _local.keySet().removeAll( _toDelete );
            _toDelete.clear();
         }
      }
      Performance.record( "refresh", System.nanoTime() - atStart );
   }

   void updateFromNetwork( ByteBuffer source ) {
      synchronized( _toUpdate ) {
         _toUpdate.add( source );
      }
   }

   void deleteFromNetwork( GUID id ) {
      synchronized( _toDelete ) {
         _toDelete.add( id );
      }
   }
}
