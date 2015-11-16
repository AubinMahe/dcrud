#include <dcrud/IRepository.h>
#include <dcrud/Shareable.h>
#include <dcrud/IRepositoryFactory.h>
#include <coll/Set.h>
#include <coll/List.h>
#include "Repositories.h"
#include <stdio.h>
#include <string.h>

typedef struct Cache_s {

   collSet                 updated;
   collSet                 deleted;
   collSet                 toUpdate;
   collSet                 toDelete;
   collMap                 local;
   char                    topic[1024];
   dcrudShareableFactory   factory;
   dcrudIRepositoryFactory network;
   unsigned int            lastInstanceId;

} Cache;

static collList caches = NULL;

static int shareableComparator( dcrudShareable * * left, dcrudShareable * * right ) {
   return (int)( *left - *right );
}

dcrudIRepository dcrudCache_init(
   const char *            topic,
   dcrudShareableFactory   factory,
   dcrudIRepositoryFactory network )
{
   Cache * cache;
   if( caches == NULL ) {
      caches = collList_reserve();
   }
   cache = (Cache *)malloc( sizeof( Cache ) );
   collList_add( caches, cache );
   cache->updated  = collSet_reserve((collComparator)shareableComparator );
   cache->deleted  = collSet_reserve((collComparator)shareableComparator );
   cache->toUpdate = collSet_reserve((collComparator)shareableComparator );
   cache->toDelete = collSet_reserve((collComparator)dcrudGUID_compareTo );
   cache->local    = collMap_reserve((collComparator)dcrudGUID_compareTo );
   strncpy( cache->topic, topic, sizeof( cache->topic ));
   cache->factory  = factory;
   cache->network  = network;
   cache->lastInstanceId = 0;
   return (dcrudIRepository)cache;
}

/* private interface, not published in public headers */
void dcrudGUID_setInstance( const dcrudGUID self, unsigned int instance );

dcrudErrorCode dcrudIRepository_create( dcrudIRepository This, dcrudShareable item ) {
   Cache *   cache = (Cache *)This;
   dcrudGUID id    = dcrudShareable_getId( item );
   if( dcrudGUID_isShared( id )) {
      char buffer[40];
      dcrudGUID_toString( id, buffer, sizeof( buffer ));
      fprintf( stderr, "Item already published: %s!\n", buffer );
      return DCRUD_ALREADY_CREATED;
   }
   dcrudGUID_setInstance( id, ++cache->lastInstanceId );
   collMap_put( cache->local, id, item );
   collSet_add( cache->updated, item );
   return DCRUD_NO_ERROR;
}

dcrudErrorCode dcrudIRepository_update( dcrudIRepository This, dcrudShareable item ) {
   Cache * cache = (Cache *)This;
   dcrudGUID id = dcrudShareable_getId( item );
   if( ! dcrudGUID_isShared( id )) {
      fprintf( stderr, "Item must be created first!\n" );
      return DCRUD_NOT_CREATED;
   }
   if( ! collMap_get( cache->local, id )) {
      char itemId[40];
      dcrudGUID_toString( id, itemId, sizeof( itemId ));
      fprintf( stderr, "Repository doesn't contains item %s to update!\n", itemId );
      return false;
   }
   collSet_add( cache->updated, item );
   return DCRUD_NO_ERROR;
}

dcrudErrorCode dcrudIRepository_publish( dcrudIRepository This ) {
   Cache * cache = (Cache *)This;
   Repositories_publish( cache->network, cache->updated, cache->deleted );
   collSet_clear( cache->updated );
   collSet_clear( cache->deleted );
   return DCRUD_NO_ERROR;
}
