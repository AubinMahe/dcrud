#include <dcrud/Shareable.h>
#include <coll/Set.h>
#include <coll/List.h>
#include <dcrud/ICache.h>
#include <dcrud/IParticipant.h>
#include "GUID_private.h"

#include <stdio.h>
#include <string.h>
#include "Network.h"

static byte NextCacheId = 1; /* cache 0 doesn't exists, it's a flag for operation */

typedef struct Cache_s {

   collSet               classes;
   collSet               updated;
   collSet               deleted;
   collSet               toUpdate;
   collSet               toDelete;
   collMapVoidPtr        local;
   unsigned int          nextInstance;
   bool                  ownershipCheck;
   dcrudIParticipant     network;
   byte                  platformId;
   byte                  execId;
   byte                  cacheId;

} Cache;

static collList caches = NULL;

dcrudICache dcrudCache_init( dcrudIParticipant network, byte platformId, byte execId ) {
   Cache * cache;
   if( caches == NULL ) {
      caches = collList_new();
   }
   cache = (Cache *)malloc( sizeof( Cache ) );
   memset( cache, 0, sizeof( Cache ));
   collList_add( caches, cache );
   cache->classes        = collSet_new((collComparator)dcrudClassID_compareTo );
   cache->updated        = collSet_new((collComparator)dcrudShareable_compareTo );
   cache->deleted        = collSet_new((collComparator)dcrudShareable_compareTo );
   cache->toUpdate       = collSet_new((collComparator)dcrudShareable_compareTo );
   cache->toDelete       = collSet_new((collComparator)dcrudGUID_compareTo );
   cache->local          = collMapVoidPtr_new((collComparator)dcrudGUID_compareTo );
   cache->nextInstance   = 1;
   cache->ownershipCheck = false;
   cache->network        = network;
   cache->platformId     = platformId;
   cache->execId         = execId;
   cache->cacheId        = NextCacheId++;
   return (dcrudICache)cache;
}

byte dcrudICache_getId( dcrudICache self ) {
   Cache * This = (Cache*)self;
   return This->cacheId;
}

static bool matches( Cache * This, byte platformId, byte execId, byte cacheId ) {
   return( platformId == This->platformId )
      && ( execId     == This->execId     )
      && ( cacheId    == This->cacheId    );
}


bool dcrudICache_owns( dcrudICache self, dcrudGUID guid ) {
   Cache * This = (Cache*)self;
   dcrudGUIDImpl * id = (dcrudGUIDImpl *)guid;
   return matches( This, id->platform, id->exec, id->cache );
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
   collMapVoidPtr_put( This->local, id, item, NULL );
   collSet_add( This->updated, item );
   return DCRUD_NO_ERROR;
}

dcrudStatus dcrudICache_update( dcrudICache This, dcrudShareable item ) {
   Cache *         cache = (Cache *)This;
   dcrudGUIDImpl * id    = (dcrudGUIDImpl *)dcrudShareable_getGUID( item );
   if( ! dcrudGUID_isShared( id )) {
      fprintf( stderr, "Item must be created first!\n" );
      return DCRUD_NOT_CREATED;
   }
   if( ! collMapVoidPtr_get( cache->local, id )) {
      char itemId[40];
      dcrudGUID_toString((dcrudGUID)id, itemId, sizeof( itemId ));
      fprintf( stderr, "Repository doesn't contains item %s to update!\n", itemId );
      return false;
   }
   collSet_add( cache->updated, item );
   return DCRUD_NO_ERROR;
}

dcrudStatus dcrudICache_publish( dcrudICache This ) {
   Cache * cache = (Cache *)This;
   Network_publish( cache->network, cache->cacheId, cache->updated, cache->deleted );
   collSet_clear( cache->updated );
   collSet_clear( cache->deleted );
   return DCRUD_NO_ERROR;
}

void dcrudICache_subscribe( dcrudICache self, dcrudClassID id ) {
   Cache * This = (Cache*)self;
   collSet_add( This->classes, id );
}
