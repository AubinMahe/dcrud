#include <dcrud/Shareable.h>
#include <coll/Set.h>
#include <coll/List.h>

#include <dcrud/ICache.h>
#include <dcrud/IParticipant.h>

#include <os/Mutex.h>
#include <os/System.h>

#include <dbg/Performance.h>

#include "GUID_private.h"
#include "Shareable_private.h"

#include <stdio.h>
#include <string.h>
#include "Network.h"

static byte NextCacheId = 1; /* cache 0 doesn't exists, it's a flag for operation */

typedef struct Cache_s {

   collSet           classes;
   osMutex           classesMutex;
   collSet           updated;
   osMutex           updatedMutex;
   collSet           deleted;
   osMutex           deletedMutex;
   collSet           toUpdate;
   osMutex           toUpdateMutex;
   collSet           toDelete;
   osMutex           toDeleteMutex;
   collMap           local;
   osMutex           localMutex;
   unsigned int      nextInstance;
   bool              ownershipCheck;
   dcrudIParticipant network;
   byte              platformId;
   byte              execId;
   byte              cacheId;

} Cache;

static collList caches = NULL;

dcrudICache dcrudCache_new( dcrudIParticipant network, byte platformId, byte execId ) {
   Cache * This = (Cache *)malloc( sizeof( Cache ));
   memset( This, 0, sizeof( Cache ));
   if( caches == NULL ) {
      caches = collList_new();
   }
   collList_add( caches, This );
   This->classes        = collSet_new((collComparator)dcrudClassID_compareTo );
   This->updated        = collSet_new((collComparator)dcrudShareable_compareTo );
   This->deleted        = collSet_new((collComparator)dcrudShareable_compareTo );
   This->toUpdate       = collSet_new((collComparator)dcrudShareable_compareTo );
   This->toDelete       = collSet_new((collComparator)dcrudGUID_compareTo );
   This->local          = collMap_new((collComparator)dcrudGUID_compareTo );
   This->nextInstance   = 1;
   This->ownershipCheck = false;
   This->network        = network;
   This->platformId     = platformId;
   This->execId         = execId;
   This->cacheId        = NextCacheId++;
   osMutex_new( &This->classesMutex  );
   osMutex_new( &This->updatedMutex  );
   osMutex_new( &This->deletedMutex  );
   osMutex_new( &This->toUpdateMutex );
   osMutex_new( &This->toDeleteMutex );
   osMutex_new( &This->localMutex    );
   return (dcrudICache)This;
}

void dcrudCache_delete( dcrudICache * This ) {
   free( *This );
   *This = NULL;
}

bool dcrudCache_matches( dcrudICache self, byte platformId, byte execId, byte cacheId ) {
   Cache * This = (Cache *)self;
   return( platformId == This->platformId )
      && ( execId     == This->execId     )
      && ( cacheId    == This->cacheId    );
}

bool dcrudICache_owns( dcrudICache self, dcrudGUID guid ) {
   dcrudGUIDImpl * id = (dcrudGUIDImpl *)guid;
   return dcrudCache_matches( self, id->platform, id->exec, id->cache );
}

void dcrudICache_setOwnership( dcrudICache self, bool enabled ) {
   Cache * This = (Cache *)self;
   This->ownershipCheck = enabled;
}

dcrudStatus dcrudICache_create( dcrudICache self, dcrudShareable item ) {
   Cache *            This = (Cache*)self;
   dcrudGUIDImpl *    id   = (dcrudGUIDImpl *)dcrudShareable_getGUID( item );

   if( dcrudGUID_isShared( id )) {
      char buffer[40];
      dcrudGUID_toString((dcrudGUID)id, buffer, sizeof( buffer ));
      fprintf( stderr, "Item already published: %s!\n", buffer );
      return DCRUD_ALREADY_CREATED;
   }
   id->platform = This->platformId;
   id->exec     = This->execId;
   id->cache    = This->cacheId;
   id->instance = This->nextInstance++;
   osMutex_take( This->localMutex );
   collMap_put( This->local, id, item, NULL );
   osMutex_release( This->localMutex );
   osMutex_take( This->updatedMutex );
   collSet_add( This->updated, item );
   osMutex_release( This->updatedMutex );
   return DCRUD_NO_ERROR;
}

dcrudShareable dcrudICache_read( dcrudICache self, dcrudGUID id ) {
   Cache * This = (Cache *)self;
   return collMap_get( This->local, id );
}

dcrudStatus dcrudICache_update( dcrudICache self, dcrudShareable item ) {
   Cache *         This = (Cache *)self;
   dcrudGUIDImpl * id   = (dcrudGUIDImpl *)dcrudShareable_getGUID( item );
   if( ! dcrudGUID_isShared( id )) {
      fprintf( stderr, "Item must be created first!\n" );
      return DCRUD_NOT_CREATED;
   }
   if( ! collMap_get( This->local, id )) {
      char itemId[40];
      dcrudGUID_toString((dcrudGUID)id, itemId, sizeof( itemId ));
      fprintf( stderr, "Repository doesn't contains item %s to update!\n", itemId );
      return false;
   }
   osMutex_take( This->updatedMutex );
   collSet_add( This->updated, item );
   osMutex_release( This->updatedMutex );
   return DCRUD_NO_ERROR;
}

dcrudStatus dcrudICache_delete( dcrudICache self, dcrudShareable item ) {
   Cache *   This = (Cache *)self;
   dcrudGUID id   = dcrudShareable_getGUID( item );
   bool      notFound;
   if( This->ownershipCheck && ! dcrudICache_owns( self, id )) {
      return DCRUD_NOT_OWNER;
   }
   osMutex_take( This->localMutex );
   notFound = collMap_remove( This->local, id, NULL );
   osMutex_release( This->localMutex );
   if( notFound ) {
      return DCRUD_NOT_IN_THIS_CACHE;
   }
   osMutex_take( This->deletedMutex );
   collSet_add( This->deleted, item );
   osMutex_release( This->deletedMutex );
   return DCRUD_NO_ERROR;
}

collMapPairs dcrudICache_values( dcrudICache self ) {
   Cache * This = (Cache *)self;
   collMapPairs pairs;

   osMutex_take( This->localMutex );
   pairs = collMap_values( This->local );
   osMutex_release( This->localMutex );
   return pairs;
}

collSet dcrudICache_select( dcrudICache self, dcrudPredicate query ) {
   Cache *      This      = (Cache *)self;
   collSet      selection = collSet_new((collComparator)dcrudShareable_compareTo );
   unsigned int size;
   collMapPairs values;
   unsigned int i;

   osMutex_take( This->localMutex );
   size   = collMap_size  ( This->local );
   values = collMap_values( This->local );
   for( i = 0; i < size; ++i ) {
      dcrudShareable item = (dcrudShareable)values[i].value;
      if( query == NULL || query( item )) {
         collSet_add( selection, item );
      }
   }
   osMutex_release( This->localMutex );
   return selection;
}

dcrudStatus dcrudICache_publish( dcrudICache self ) {
   Cache * This = (Cache *)self;
   osMutex_take   ( This->updatedMutex );
   osMutex_take   ( This->deletedMutex );
   Network_publish( This->network, This->cacheId, This->updated, This->deleted );
   collSet_clear  ( This->updated );
   collSet_clear  ( This->deleted );
   osMutex_release( This->deletedMutex );
   osMutex_release( This->updatedMutex );
   return DCRUD_NO_ERROR;
}

void dcrudICache_subscribe( dcrudICache self, dcrudClassID id ) {
   Cache * This = (Cache*)self;
   collSet_add( This->classes, id );
}

void dcrudICache_refresh( dcrudICache self ) {
   Cache *       This    = (Cache*)self;
   uint64_t      atStart = osSystem_nanotime();
   collSetValues toUpdate;
   collSetValues toDelete;
   unsigned int  size;
   unsigned int  i;

   osMutex_take( This->localMutex );
   osMutex_take( This->toUpdateMutex );
   toUpdate = collSet_values( This->toUpdate );
   size     = collSet_size  ( This->toUpdate );
   for( i = 0; i < size; ++i ) {
      dcrudGUID      id;
      dcrudShareable t;
      ioByteBuffer   update = toUpdate[i];
      dcrudClassID   classId;

      dcrudGUID_unserialize   ( update, &id );
      dcrudClassID_unserialize( update, &classId );
      t = collMap_get( This->local, id );
      if( t == NULL ) {
         dcrudShareable item = Network_newInstance( This->network, update );
         if( item != NULL ) {
            dcrudGUID guid = dcrudShareable_getGUID( item );
            dcrudGUID_set( guid, id );
            collMap_put( This->local, id, item, NULL );
         }
         else {
            char bufferClass[1024];
            char bufferGUID [1024];
            dcrudClassID_toString( classId, bufferClass, sizeof( bufferClass ));
            dcrudGUID_toString   ( id     , bufferGUID , sizeof( bufferGUID  ));
            fprintf( stderr, "Unknown %s of %s\n", bufferClass, bufferGUID );
         }
      }
      else if( ! This->ownershipCheck || ! dcrudICache_owns( self, id )) {
         dcrudShareableImpl * impl = (dcrudShareableImpl *)t;
         impl->unserialize( t, update );
      }
   }
   collSet_clear( This->toUpdate );
   osMutex_release( This->toUpdateMutex );

   osMutex_take( This->toDeleteMutex );
   toDelete = collSet_values( This->toDelete );
   size     = collSet_size  ( This->toDelete );
   for( i = 0; i < size; ++i ) {
      collMap_remove( This->local, toDelete[i], NULL );
   }
   collSet_clear( This->toDelete );
   osMutex_release( This->toDeleteMutex );
   osMutex_release( This->localMutex );
   dbgPerformance_record( "refresh", osSystem_nanotime() - atStart );
}

void dcrudCache_updateFromNetwork( dcrudICache self, ioByteBuffer source ) {
   Cache * This = (Cache*)self;
   osMutex_take( This->toUpdateMutex );
   collSet_add( This->toUpdate, source );
}

void dcrudCache_deleteFromNetwork( dcrudICache self, dcrudGUID id ) {
   Cache * This = (Cache*)self;
   osMutex_take( This->toDeleteMutex );
   collSet_add( This->toDelete, id );
   osMutex_release( This->toDeleteMutex );
}
