#include <coll/Map.h>
#include "magic.h"
#include "poolSizes.h"

#include <util/Pool.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct collMapImpl_s {

   unsigned       magic;
   collComparator cmp;
   unsigned       count;
#ifdef STATIC_ALLOCATION
   collMapPair    pairs[collMap_ITEM_MAX_COUNT];
#else
   unsigned       limit;
   collMapPair *  pairs;
#endif

} collMapImpl;

UTIL_DEFINE_SAFE_CAST( collMap     )
UTIL_POOL_DECLARE    ( collMapImpl )

typedef int ( * PVoidComparator )( const void *, const void * );

utilStatus collMap_new( collMap * self, collComparator cmp ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( NULL == self ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      collMapImpl * This = NULL;
      UTIL_ALLOCATE_ADT( collMap, self, This  );
      if( status == UTIL_STATUS_NO_ERROR ) {
         This->cmp   = cmp;
         This->count = 0;
#ifndef STATIC_ALLOCATION
         This->limit = 100;
         This->pairs = (collMapPair*)malloc( This->limit * sizeof( collMapPair));
         if( This->pairs == NULL ) {
            free( This );
            status = UTIL_STATUS_TOO_MANY;
            *self  = NULL;
         }
#endif
      }
   }
   return status;
}

utilStatus collMap_delete( collMap * self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( NULL == self ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      collMapImpl * This = collMap_safeCast( *self, &status );
      if( UTIL_STATUS_NO_ERROR == status ) {
         (void)This;
#ifndef STATIC_ALLOCATION
         free( This->pairs );
#endif
         UTIL_RELEASE( collMapImpl );
         *self  = NULL;
      }
   }
   return status;
}

utilStatus collMap_clear( collMap self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   collMapImpl * This = collMap_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      This->count = 0;
#ifndef STATIC_ALLOCATION
      This->limit = 100;
      This->pairs = (collMapPair*)realloc( This->pairs, This->limit * sizeof( collMapPair));
#endif
   }
   return status;
}

utilStatus collMap_put(
   collMap       self,
   collMapKey    key,
   collMapValue  value,
   collMapPair * previousPair )
{
   utilStatus    status  = UTIL_STATUS_NO_ERROR;
   collMapImpl * This    = collMap_safeCast( self, &status );
   collMapPair * oldPair = NULL;
   collMapPair   newPair;
   if( previousPair ) {
      previousPair->key   = NULL;
      previousPair->value = NULL;
   }
   if( status != UTIL_STATUS_NO_ERROR ) {
      return status;
   }
   newPair.key   = key;
   newPair.value = NULL;
   oldPair =
      bsearch(
         &newPair, This->pairs, This->count, sizeof( collMapPair),
         (PVoidComparator)This->cmp );
   if( oldPair ) {
      if( previousPair ) {
         *previousPair = *oldPair;
      }
      oldPair->value = value;
      return UTIL_STATUS_NO_ERROR;
   }
#ifdef STATIC_ALLOCATION
   if( This->count == collMap_ITEM_MAX_COUNT ) {
      return UTIL_STATUS_TOO_MANY;
   }
#else
   if( This->count == This->limit ) {
      This->limit += 100;
      This->pairs = (collMapPair*)
         realloc( This->pairs, This->limit * sizeof( collMapPair));
      if( This->pairs == NULL ) {
         This->count = 0;
         This->limit = 0;
         return UTIL_STATUS_TOO_MANY;
      }
   }
#endif
   oldPair = (collMapPair*)(This->pairs + This->count++);
   oldPair->key   = key;
   oldPair->value = value;
   qsort( This->pairs, This->count, sizeof( collMapPair), (PVoidComparator)This->cmp );
   return UTIL_STATUS_NO_ERROR;
}

utilStatus collMap_remove( collMap self, collMapKey key, collMapPair * previousPair ) {
   utilStatus    status = UTIL_STATUS_NO_ERROR;
   collMapImpl * This   = collMap_safeCast( self, &status );
   collMapPair   search;
   collMapPair * prev;
   unsigned      moved;
   if( previousPair ) {
      previousPair->key   = NULL;
      previousPair->value = NULL;
   }
   if( status == UTIL_STATUS_NO_ERROR ) {
      search.key   = key;
      search.value = NULL;
      prev =
         bsearch(
            &search,
            This->pairs,
            This->count,
            sizeof( collMapPair),
            (PVoidComparator)This->cmp );
      if( prev ) {
         if( previousPair ) {
            previousPair->key   = prev->key;
            previousPair->value = prev->value;
         }
         moved = ( This->pairs + This->count - 1 - prev );
         memmove( prev, prev + 1, moved*sizeof( collMapPair));
         --This->count;
         status = UTIL_STATUS_NO_ERROR;
      }
      else {
         status = UTIL_STATUS_NOT_FOUND;
      }
   }
   return status;
}

utilStatus collMap_get( collMap self, collMapKey key, void * arg ) {
   utilStatus     status = UTIL_STATUS_NO_ERROR;
   collMapImpl *  This   = collMap_safeCast( self, &status );
   collMapPair    search;
   collMapPair *  pair;
   collMapValue * result = (collMapValue *)arg;
   if( arg == NULL ) {
      return UTIL_STATUS_NULL_ARGUMENT;
   }
   if( status != UTIL_STATUS_NO_ERROR ) {
      return status;
   }
   search.key   = key;
   search.value = NULL;
   pair =
      bsearch(
         &search,
         This->pairs,
         This->count,
         sizeof( collMapPair),
         (PVoidComparator)This->cmp );
   if( pair ) {
      *result = pair->value;
      status  = UTIL_STATUS_NO_ERROR;
   }
   else {
      *result = NULL;
      status  = UTIL_STATUS_NOT_FOUND;
   }
   return status;
}

utilStatus collMap_size( collMap self, unsigned int * result ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( result == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      collMapImpl * This = collMap_safeCast( self, &status );
      if( status == UTIL_STATUS_NO_ERROR ) {
         *result = This->count;
      }
      else {
         *result = 0U;
      }
   }
   return status;
}

utilStatus collMap_hasKey( collMap self, collMapKey key, bool * result ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( result == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      collMapValue value  = NULL;
      utilStatus   status = collMap_get( self, key, &value );
      *result = false;
      if( status == UTIL_STATUS_NO_ERROR ) {
         *result = ( value != NULL );
      }
   }
   return status;
}

utilStatus collMap_foreach(
   collMap             self,
   collForeachFunction fn,
   void *              userData,
   collForeachResult * result   )
{
   utilStatus status = UTIL_STATUS_NO_ERROR;
   collMapImpl * This = collMap_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      collForeach context;
      memset( &context, 0, sizeof( context ));
      context.user = userData;
      for( context.index = 0;
           ( UTIL_STATUS_NO_ERROR == status ) && ( context.index < This->count );
           ++context.index )
      {
         collMapPair * pair = This->pairs + context.index;
         context.key   = pair->key;
         context.value = pair->value;
         status = fn( &context );
      }
      if( result ) {
         *result = context.retVal;
      }
   }
   return status;
}

utilStatus collMap_values( collMap self, collMapValues * result ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( result == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      collMapImpl * This = collMap_safeCast( self, &status );
      if( status == UTIL_STATUS_NO_ERROR ) {
         *result = This->pairs;
      }
   }
   return status;
}
