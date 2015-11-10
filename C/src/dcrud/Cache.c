#include <dcrud/IRepository.h>
#include <dcrud/Shareable.h>
#include <dcrud/IRepositoryFactory.h>
#include <coll/Set.h>
#include <coll/List.h>
#include "Repositories.h"
#include <stdio.h>

typedef struct Cache_s {

   collSet                 updated;
   collSet                 deleted;
   collSet                 toUpdate;
   collSet                 toDelete;
   collMap                 local;
   int                     sourceId;
   bool                    producer;
   dcrudShareableFactory   factory;
   dcrudIRepositoryFactory network;
   int                     lastInstanceId;

} Cache;

static collList caches = NULL;

static int shareableComparator( dcrudShareable * * left, dcrudShareable * * right ) {
   return (int)( *left - *right );
}

dcrudIRepository dcrudCache_init(
   int                     sourceId,
   bool                    producer,
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
   cache->sourceId = sourceId;
   cache->producer = producer;
   cache->factory  = factory;
   cache->network  = network;
   cache->lastInstanceId = 0;
   return (dcrudIRepository)cache;
}

/* private interface, not published in public headers */
dcrudGUID dcrudGUID_init( int source, int instance );
void dcrudShareable_setId( dcrudShareable self, dcrudGUID id );

void dcrudIRepository_create( dcrudIRepository This, dcrudShareable item ) {
   Cache * cache = (Cache *)This;
   dcrudGUID id = dcrudShareable_getId( item );
   if( id ) {
      char buffer[16];
      dcrudGUID_toString( id, buffer, sizeof( buffer ));
      fprintf( stderr, "Item already published: %s!\n", buffer );
      return;
   }
   if( ! cache->producer ) {
      char buffer[16];
      dcrudGUID_toString( id, buffer, sizeof( buffer ) );
      fprintf( stderr, "Only owner can create; %s!\n", buffer );
   }
   dcrudShareable_setId( item, dcrudGUID_init( cache->sourceId, ++cache->lastInstanceId ));
   id = dcrudShareable_getId( item );
   collMap_put( cache->local, id, item );
   collSet_add( cache->updated, item );
}

bool dcrudIRepository_update( dcrudIRepository This, dcrudShareable item ) {
   Cache * cache = (Cache *)This;
   dcrudGUID id = dcrudShareable_getId( item );
   if( !id || !dcrudGUID_isValid( id )) {
      fprintf( stderr, "Item must be created first!\n" );
      return false;
   }
   if( ! cache->producer ) {
      fprintf( stderr, "Only owner can update!\n" );
      return false;
   }
   if( ! collMap_get( cache->local, id )) {
      char itemId[10];
      dcrudGUID_toString( id, itemId, sizeof( itemId ));
      fprintf( stderr, "Repository doesn't contains item %s to update!\n", itemId );
      return false;
   }
   collSet_add( cache->updated, item );
   return true;
}

void dcrudIRepository_publish( dcrudIRepository This ) {
   Cache * cache = (Cache *)This;
   Repositories_publish( cache->network, cache->updated, cache->deleted );
   collSet_clear( cache->updated );
   collSet_clear( cache->deleted);
}
