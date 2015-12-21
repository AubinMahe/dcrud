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

#include "ParticipantImpl.h"

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
   ParticipantImpl * participant;

} Cache;

dcrudICache dcrudCache_new( ParticipantImpl * participant ) {
   Cache * This = (Cache *)malloc( sizeof( Cache ));
   memset( This, 0, sizeof( Cache ));
   This->classes        = collSet_new((collComparator)dcrudClassID_compareTo );
   This->updated        = collSet_new((collComparator)dcrudShareable_compareTo );
   This->deleted        = collSet_new((collComparator)dcrudShareable_compareTo );
   This->toUpdate       = collSet_new((collComparator)dcrudShareable_compareTo );
   This->toDelete       = collSet_new((collComparator)dcrudGUID_compareTo );
   This->local          = collMap_new((collComparator)dcrudGUID_compareTo );
   This->nextInstance   = 1;
   This->ownershipCheck = false;
   This->participant    = participant;
   osMutex_new( &This->classesMutex  );
   osMutex_new( &This->updatedMutex  );
   osMutex_new( &This->deletedMutex  );
   osMutex_new( &This->toUpdateMutex );
   osMutex_new( &This->toDeleteMutex );
   osMutex_new( &This->localMutex    );
   return (dcrudICache)This;
}

void dcrudCache_delete( dcrudICache * self ) {
   Cache * This = (Cache *)*self;
   if( This ) {
      collSet_delete( &This->classes       );
      collSet_delete( &This->updated       );
      collSet_delete( &This->deleted       );
      collSet_delete( &This->toUpdate      );
      collSet_delete( &This->toDelete      );
      collMap_delete( &This->local         );
      osMutex_delete( &This->classesMutex  );
      osMutex_delete( &This->updatedMutex  );
      osMutex_delete( &This->deletedMutex  );
      osMutex_delete( &This->toUpdateMutex );
      osMutex_delete( &This->toDeleteMutex );
      osMutex_delete( &This->localMutex    );
      free( This );
      *self = NULL;
   }
}

bool dcrudICache_owns( dcrudICache self, dcrudGUID guid ) {
   Cache *         This = (Cache *)self;
   dcrudGUIDImpl * id   = (dcrudGUIDImpl *)guid;
   return This->participant->publisherId == id->publisher;
}

void dcrudICache_setOwnership( dcrudICache self, bool enabled ) {
   Cache * This = (Cache *)self;
   This->ownershipCheck = enabled;
}

dcrudStatus dcrudICache_create( dcrudICache self, dcrudShareable item ) {
   Cache *   This = (Cache*)self;
   dcrudGUID id   = dcrudShareable_getGUID( item );

   if( dcrudGUID_isShared( id )) {
      char buffer[40];
      dcrudGUID_toString((dcrudGUID)id, buffer, sizeof( buffer ));
      fprintf( stderr, "%s:%d:Item already published: %s!\n", __FILE__, __LINE__, buffer );
      return DCRUD_ALREADY_CREATED;
   }
   dcrudGUID_init( id, This->participant->publisherId, This->nextInstance++ );
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
   Cache *   This = (Cache *)self;
   dcrudGUID id   = dcrudShareable_getGUID( item );
   if( ! dcrudGUID_isShared( id )) {
      fprintf( stderr, "%s:%d:Item must be created first!\n", __FILE__, __LINE__ );
      return DCRUD_NOT_CREATED;
   }
   if( ! collMap_get( This->local, id )) {
      char itemId[40];
      dcrudGUID_toString((dcrudGUID)id, itemId, sizeof( itemId ));
      fprintf( stderr, "%s:%d:Repository doesn't contains item %s to update!\n",
         __FILE__, __LINE__, itemId );
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

collForeachResult dcrudICache_foreach( dcrudICache self, collForeachFunction fn, void * uData ) {
   Cache *           This = (Cache *)self;
   collForeachResult result;

   osMutex_take( This->localMutex );
   result = collMap_foreach( This->local, fn, uData );
   osMutex_release( This->localMutex );
   return result;
}

typedef struct selectionCtxt_s {

   dcrudPredicate query;
   collSet        selection;

} selectionCtxt;

static bool selectItem( collForeach * context ) {
   collMapPair *   pair = (collMapPair *  )context->item;
   selectionCtxt * ctxt = (selectionCtxt *)context->user;
   dcrudShareable  item = (dcrudShareable )pair->value;

   if( ctxt->query == NULL || ctxt->query( item )) {
      collSet_add( ctxt->selection, item );
   }
   return true;
}

collSet dcrudICache_select( dcrudICache self, dcrudPredicate query ) {
   Cache *       This = (Cache *)self;
   selectionCtxt ctxt;

   ctxt.query     = query;
   ctxt.selection = collSet_new((collComparator)dcrudShareable_compareTo );
   osMutex_take   ( This->localMutex );
   collMap_foreach( This->local, selectItem, &ctxt );
   osMutex_release( This->localMutex );
   return ctxt.selection;
}

dcrudStatus dcrudICache_publish( dcrudICache self ) {
   Cache * This = (Cache *)self;

   osMutex_take   ( This->updatedMutex );
   ParticipantImpl_publishUpdated( This->participant, This->updated );
   collSet_clear  ( This->updated );
   osMutex_release( This->updatedMutex );

   osMutex_take   ( This->deletedMutex );
   ParticipantImpl_publishDeleted( This->participant, This->deleted );
   collSet_clear  ( This->deleted );
   osMutex_release( This->deletedMutex );

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

      dcrudGUID_unserialize( update, &id );
      t = collMap_get( This->local, id );
      if( t == NULL ) {
         dcrudShareable item = ParticipantImpl_newInstance( This->participant, update );
         if( item != NULL ) {
            dcrudGUID guid = dcrudShareable_getGUID( item );
            dcrudGUID_set( guid, id );
            collMap_put( This->local, id, item, NULL );
         }
         else {
            char bufferClass[1024];
            char bufferGUID [1024];
            dcrudClassID classId;
            dcrudClassID_unserialize( update, &classId );
            dcrudClassID_toString( classId, bufferClass, sizeof( bufferClass ));
            dcrudGUID_toString   ( id     , bufferGUID , sizeof( bufferGUID  ));
            fprintf( stderr, "%s:%d:Unknown %s of %s\n",
               __FILE__, __LINE__, bufferClass, bufferGUID );
         }
      }
      else if( ! This->ownershipCheck || ! dcrudICache_owns( self, id )) {
         dcrudShareableImpl * impl = (dcrudShareableImpl *)t;
         impl->unserialize( t, update );
      }
   }
   collSet_clear  ( This->toUpdate );
   osMutex_release( This->toUpdateMutex );

   osMutex_take   ( This->toDeleteMutex );
   toDelete = collSet_values( This->toDelete );
   size     = collSet_size  ( This->toDelete );
   for( i = 0; i < size; ++i ) {
      collMap_remove( This->local, toDelete[i], NULL );
   }
   collSet_clear  ( This->toDelete );
   osMutex_release( This->toDeleteMutex );
   osMutex_release( This->localMutex );
   dbgPerformance_record( "refresh", osSystem_nanotime() - atStart );
}

void dcrudCache_updateFromNetwork( dcrudICache self, ioByteBuffer source ) {
   Cache * This = (Cache*)self;
   osMutex_take   ( This->toUpdateMutex );
   collSet_add    ( This->toUpdate, source );
   osMutex_release( This->toUpdateMutex );
}

void dcrudCache_deleteFromNetwork( dcrudICache self, dcrudGUID id ) {
   Cache * This = (Cache*)self;
   osMutex_take   ( This->toDeleteMutex );
   collSet_add    ( This->toDelete, id );
   osMutex_release( This->toDeleteMutex );
}
