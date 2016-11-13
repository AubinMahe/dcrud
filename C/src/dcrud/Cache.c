#include <dcrud/Shareable.h>
#include <dcrud/ICache.h>
#include <dcrud/IParticipant.h>

#include "magic.h"
#include "poolSizes.h"
#include "GUID_private.h"
#include "Shareable_private.h"
#include "ParticipantImpl.h"

#include <util/Performance.h>
#include <util/Pool.h>

#include <coll/Set.h>
#include <coll/List.h>

#include <os/Mutex.h>
#include <os/System.h>

#include <stdio.h>
#include <string.h>

typedef struct dcrudCacheImpl_s {

   unsigned int      magic;
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
   dcrudIParticipantImpl * participant;

} dcrudICacheImpl;

UTIL_DEFINE_SAFE_CAST( dcrudICache     )
UTIL_POOL_DECLARE    ( dcrudICacheImpl )

utilStatus dcrudCache_new( dcrudICache * self, dcrudIParticipantImpl * participant ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( NULL == self ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudICacheImpl * This = NULL;
      UTIL_ALLOCATE_ADT( dcrudICache, self, This  );
      if( UTIL_STATUS_NO_ERROR == status ) {
         if( UTIL_STATUS_NO_ERROR == status ) {
            status = collSet_new( &This->classes , (collComparator)dcrudClassID_compareTo   );
         }
         if( UTIL_STATUS_NO_ERROR == status ) {
            status = collSet_new( &This->updated , (collComparator)dcrudShareable_compareTo );
         }
         if( UTIL_STATUS_NO_ERROR == status ) {
            status = collSet_new( &This->deleted , (collComparator)dcrudShareable_compareTo );
         }
         if( UTIL_STATUS_NO_ERROR == status ) {
            status = collSet_new( &This->toUpdate, (collComparator)collPointerCompare );
         }
         if( UTIL_STATUS_NO_ERROR == status ) {
            status = collSet_new( &This->toDelete, (collComparator)dcrudGUID_compareTo      );
         }
         if( UTIL_STATUS_NO_ERROR == status ) {
            status = collMap_new( &This->local   , (collComparator)dcrudGUID_compareTo      );
         }
         if( UTIL_STATUS_NO_ERROR == status ) {
            status = osMutex_new( &This->classesMutex  );
         }
         if( UTIL_STATUS_NO_ERROR == status ) {
            status = osMutex_new( &This->updatedMutex  );
         }
         if( UTIL_STATUS_NO_ERROR == status ) {
            status = osMutex_new( &This->deletedMutex  );
         }
         if( UTIL_STATUS_NO_ERROR == status ) {
            status = osMutex_new( &This->toUpdateMutex );
         }
         if( UTIL_STATUS_NO_ERROR == status ) {
            status = osMutex_new( &This->toDeleteMutex );
         }
         if( UTIL_STATUS_NO_ERROR == status ) {
            status = osMutex_new( &This->localMutex    );
         }
         if( UTIL_STATUS_NO_ERROR == status ) {
            This->nextInstance   = 1;
            This->ownershipCheck = false;
            This->participant    = participant;
         }
         else {
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
            UTIL_RELEASE( dcrudICacheImpl )
         }
      }
   }
   return status;
}

static utilStatus deleteShareable( collForeach * context ) {
   return dcrudShareable_delete((dcrudShareable *)&context->value );
}

static utilStatus releaseByteBuffer( collForeach * context ) {
   return ioByteBuffer_delete((ioByteBuffer *)&context->value );
}

static utilStatus releaseGUID( collForeach * context ) {
   return dcrudGUID_delete((dcrudGUID *)&context->value );
}

utilStatus dcrudCache_delete( dcrudICache * self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( NULL == self ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudICacheImpl * This = dcrudICache_safeCast( *self, &status );
      if( UTIL_STATUS_NO_ERROR == status ) {
         collMap_foreach( This->local, deleteShareable, NULL );
         collSet_foreach( This->toUpdate, releaseByteBuffer, NULL );
         collSet_foreach( This->toDelete, releaseGUID, NULL );
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
         UTIL_RELEASE( dcrudICacheImpl );
      }
   }
   return status;
}

utilStatus dcrudICache_owns( dcrudICache self, dcrudGUID guid, bool * owns ) {
   utilStatus       status = UTIL_STATUS_NO_ERROR;
   dcrudICacheImpl * This   = dcrudICache_safeCast( self, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      status = dcrudGUID_isOwnedBy( guid, This->participant->publisherId, owns );
   }
   return status;
}

utilStatus dcrudICache_setOwnership( dcrudICache self, bool enabled ) {
   utilStatus        status = UTIL_STATUS_NO_ERROR;
   dcrudICacheImpl * This   = dcrudICache_safeCast( self, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      This->ownershipCheck = enabled;
   }
   return status;
}

utilStatus dcrudICache_create( dcrudICache self, dcrudShareable item ) {
   utilStatus       status = UTIL_STATUS_NO_ERROR;
   dcrudICacheImpl * This   = dcrudICache_safeCast( self, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      dcrudGUID id;
      bool      shared = false;
      status = dcrudShareable_getGUID( item, &id );
      if( UTIL_STATUS_NO_ERROR == status ) {
         status = dcrudGUID_isShared( id, &shared );
      }
      if( UTIL_STATUS_NO_ERROR == status ) {
         if( shared ) {
            char buffer[40];
            dcrudGUID_toString((dcrudGUID)id, buffer, sizeof( buffer ));
            status = UTIL_STATUS_DUPLICATE;
         }
         else {
            dcrudGUID_init( id, This->participant->publisherId, This->nextInstance++ );
            osMutex_take( This->localMutex );
            collMap_put( This->local, id, item, NULL );
            osMutex_release( This->localMutex );
            osMutex_take( This->updatedMutex );
            collSet_add( This->updated, item );
            osMutex_release( This->updatedMutex );
         }
      }
   }
   return status;
}

utilStatus dcrudICache_read( dcrudICache self, dcrudGUID id, dcrudShareable * target ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( target == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudICacheImpl * This = dcrudICache_safeCast( self, &status );
      if( UTIL_STATUS_NO_ERROR == status ) {
         void * value = NULL;
         status = collMap_get( This->local, id, &value );
         *target = value;
      }
   }
   return status;
}

utilStatus dcrudICache_update( dcrudICache self, dcrudShareable item ) {
   utilStatus        status = UTIL_STATUS_NO_ERROR;
   dcrudICacheImpl * This   = dcrudICache_safeCast( self, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      dcrudGUID guid = NULL;
      status = dcrudShareable_getGUID( item, &guid );
      if( UTIL_STATUS_NO_ERROR == status ) {
         bool shared = false;
         status = dcrudGUID_isShared( guid, &shared );
         if( UTIL_STATUS_NO_ERROR == status ) {
            if( shared ) {
               bool exists = false;
               status = collMap_hasKey( This->local, guid, &exists );
               if(( UTIL_STATUS_NO_ERROR == status ) && exists ) {
                  status = osMutex_take( This->updatedMutex );
                  if( UTIL_STATUS_NO_ERROR == status ) {
                     status = collSet_add( This->updated, item );
                     osMutex_release( This->updatedMutex );
                  }
               }
            }
            else {
               status = UTIL_STATUS_ILLEGAL_STATE;
            }
         }
      }
   }
   return status;
}

utilStatus dcrudICache_delete( dcrudICache self, dcrudShareable item ) {
   utilStatus       status = UTIL_STATUS_NO_ERROR;
   dcrudICacheImpl * This   = dcrudICache_safeCast( self, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      dcrudGUID id;
      status = dcrudShareable_getGUID( item, &id );
      if( This->ownershipCheck ) {
         bool isOwner = false;
         status = dcrudICache_owns( self, id, &isOwner );
         if(( status == UTIL_STATUS_NO_ERROR ) && ( ! isOwner )) {
            status = UTIL_STATUS_ILLEGAL_STATE;
         }
      }
      if( UTIL_STATUS_NO_ERROR == status ) {
         osMutex_take( This->localMutex );
         status = collMap_remove( This->local, id, NULL );
         osMutex_release( This->localMutex );
         if( UTIL_STATUS_NO_ERROR == status ) {
            osMutex_take( This->deletedMutex );
            status = collSet_add( This->deleted, item );
            osMutex_release( This->deletedMutex );
         }
      }
   }
   return status;
}

utilStatus dcrudICache_foreach( dcrudICache self, collForeachFunction fn, void * uData ) {
   utilStatus       status = UTIL_STATUS_NO_ERROR;
   dcrudICacheImpl * This   = dcrudICache_safeCast( self, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      osMutex_take( This->localMutex );
      status = collMap_foreach( This->local, fn, uData );
      osMutex_release( This->localMutex );
   }
   return status;
}

typedef struct selectionCtxt_s {

   dcrudPredicate query;
   collSet        selection;

} selectionCtxt;

static utilStatus selectItem( collForeach * context ) {
   selectionCtxt * ctxt   = (selectionCtxt *)context->user;
   dcrudShareable  item   = (dcrudShareable )context->value;
   utilStatus      status = UTIL_STATUS_NO_ERROR;

   if( ctxt->query == NULL || ctxt->query( item )) {
      status = collSet_add( ctxt->selection, item );
   }
   return status;
}

utilStatus dcrudICache_select( dcrudICache self, dcrudPredicate query, collSet result ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( NULL == result ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudICacheImpl * This = dcrudICache_safeCast( self, &status );
      if( UTIL_STATUS_NO_ERROR == status ) {
         selectionCtxt ctxt;
         ctxt.query     = query;
         ctxt.selection = result;
         CHK(__FILE__,__LINE__,osMutex_take( This->localMutex ))
         status = collMap_foreach( This->local, selectItem, &ctxt );
         osMutex_release( This->localMutex );
      }
   }
   return status;
}

static utilStatus releaseShareable( collForeach * context ) {
   dcrudShareable item = (dcrudShareable)context->value;
   return dcrudShareable_delete( &item );
}

utilStatus dcrudICache_publish( dcrudICache self ) {
   utilStatus        status = UTIL_STATUS_NO_ERROR;
   dcrudICacheImpl * This   = dcrudICache_safeCast( self, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      CHK(__FILE__,__LINE__,osMutex_take( This->updatedMutex ))
      status = dcrudIParticipantImpl_publishUpdated( This->participant, This->updated );
      collSet_clear  ( This->updated );
      osMutex_release( This->updatedMutex );
      if( UTIL_STATUS_NO_ERROR == status ) {
         CHK(__FILE__,__LINE__,osMutex_take( This->deletedMutex ))
         status = dcrudIParticipantImpl_publishDeleted( This->participant, This->deleted );
         if( UTIL_STATUS_NO_ERROR == status ) {
            status = collSet_foreach( This->deleted, releaseShareable, NULL );
            collSet_clear( This->deleted );
         }
         osMutex_release( This->deletedMutex );
      }
   }
   return status;
}

utilStatus dcrudICache_subscribe( dcrudICache self, dcrudClassID id ) {
   utilStatus       status = UTIL_STATUS_NO_ERROR;
   dcrudICacheImpl * This   = dcrudICache_safeCast( self, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      if( ! collSet_add( This->classes, id )) {
         status = UTIL_STATUS_TOO_MANY;
      }
   }
   return status;
}

utilStatus dcrudICache_refresh( dcrudICache self ) {
   utilStatus        status   = UTIL_STATUS_NO_ERROR;
   dcrudICacheImpl * This     = dcrudICache_safeCast( self, &status );
   uint64_t          atStart  = osSystem_nanotime();
   collSetValues     toUpdate = NULL;
   collSetValues     toDelete = NULL;
   unsigned int      size     = 0U;
   unsigned int      i        = 0U;
   if( UTIL_STATUS_NO_ERROR == status ) {
      return status;
   }
   CHK(__FILE__,__LINE__,osMutex_take( This->localMutex ))
   status = osMutex_take( This->toUpdateMutex );
   if( UTIL_STATUS_NO_ERROR == status ) {
      status = collSet_values( This->toUpdate, &toUpdate );
   }
   if( UTIL_STATUS_NO_ERROR == status ) {
      status = collSet_size  ( This->toUpdate, &size     );
   }
   for( i = 0; i < size && ( status == UTIL_STATUS_NO_ERROR ); ++i ) {
      dcrudGUID    guid   = NULL;
      ioByteBuffer update = toUpdate[i];
      status = dcrudGUID_unserialize( &guid, update );
      if( UTIL_STATUS_NO_ERROR == status ) {
         dcrudShareable t = NULL;
         status = collMap_get( This->local, guid, (void**)&t );
         if(( status == UTIL_STATUS_NO_ERROR ) && ( t == NULL )) {
            dcrudShareable item = NULL;
            status = dcrudIParticipantImpl_newInstance( This->participant, update, NULL, &item );
            if( item != NULL ) {
               status = dcrudShareable_getGUID( item, &guid );
               status = collMap_put( This->local, guid, item, NULL );
            }
            else {
               char         szClass[1024];
               char         szGUID [1024];
               dcrudClassID classId = NULL;

               dcrudClassID_unserialize( &classId, update );
               dcrudClassID_toString( classId, szClass, sizeof( szClass ));
               dcrudClassID_delete( &classId );
               dcrudGUID_toString   ( guid   , szGUID , sizeof( szGUID  ));
               fprintf( stderr, "%s:%d:Warning:unknown %s of %s\n",
                  __FILE__, __LINE__, szGUID, szClass );
            }
         }
         else {
            bool owns = false;
            if( This->ownershipCheck ) {
               status = dcrudICache_owns( self, guid, &owns );
            }
            if( ! owns ) {
               dcrudShareableImpl * impl = (dcrudShareableImpl *)t;
               status = impl->factory->unserialize( t, update );
            }
         }
      }
      status = ioByteBuffer_delete( &update );
   }
   collSet_clear  ( This->toUpdate );
   osMutex_release( This->toUpdateMutex );
   if( UTIL_STATUS_NO_ERROR == status ) {
      osMutex_take   ( This->toDeleteMutex );
      status = collSet_values( This->toDelete, &toDelete );
      status = collSet_size  ( This->toDelete, &size     );
      for( i = 0; i < size; ++i ) {
         dcrudGUID guid = toDelete[i];
         collMap_remove( This->local, guid, NULL );
         dcrudGUID_delete( &guid );
      }
      collSet_clear  ( This->toDelete );
      osMutex_release( This->toDeleteMutex );
   }
   osMutex_release( This->localMutex );
   utilPerformance_record( "refresh", osSystem_nanotime() - atStart );
   return status;
}

utilStatus dcrudCache_updateFromNetwork( dcrudICache self, ioByteBuffer source ) {
   utilStatus        status = UTIL_STATUS_NO_ERROR;
   dcrudICacheImpl * This   = dcrudICache_safeCast( self, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      osMutex_take   ( This->toUpdateMutex );
      collSet_add    ( This->toUpdate, source );
      osMutex_release( This->toUpdateMutex );
   }
   return status;
}

utilStatus dcrudCache_deleteFromNetwork( dcrudICache self, dcrudGUID id ) {
   utilStatus        status = UTIL_STATUS_NO_ERROR;
   dcrudICacheImpl * This   = dcrudICache_safeCast( self, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      osMutex_take   ( This->toDeleteMutex );
      collSet_add    ( This->toDelete, id );
      osMutex_release( This->toDeleteMutex );
   }
   return status;
}
