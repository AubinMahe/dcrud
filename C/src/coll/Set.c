#include <coll/Set.h>
#include <util/Pool.h>
#include "magic.h"
#include "poolSizes.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct collSetImpl_s {

   unsigned       magic;
   unsigned       count;
#ifdef STATIC_ALLOCATION
   collSetItem    items[collSet_ITEM_MAX_COUNT];
#else
   unsigned       limit;
   collSetItem *  items;
#endif
   collComparator cmp;

} collSetImpl;

typedef int ( * PVoidComparator )( const void *, const void * );

UTIL_DEFINE_SAFE_CAST( collSet     )
UTIL_POOL_DECLARE    ( collSetImpl )

utilStatus collSet_new( collSet * self, collComparator cmp ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( NULL == self ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      collSetImpl * This = NULL;
      UTIL_ALLOCATE_ADT( collSet, self, This  );
      if( status == UTIL_STATUS_NO_ERROR ) {
         This->count = 0;
         This->cmp   = cmp;
#ifndef STATIC_ALLOCATION
         This->limit = 100;
         This->items = (collSetItem *)malloc( This->limit * sizeof( collSetItem ));
         if( This->items == NULL ) {
            free( This );
            status = UTIL_STATUS_TOO_MANY;
            This   = NULL;
            *self  = NULL;
         }
#endif
      }
   }
   return status;
}

utilStatus collSet_delete( collSet * self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( NULL == self ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      collSetImpl * This = collSet_safeCast( *self, &status );
      if( UTIL_STATUS_NO_ERROR == status ) {
         (void)This;
#ifndef STATIC_ALLOCATION
         free( This->items );
#endif
         UTIL_RELEASE( collSetImpl );
         *self  = NULL;
      }
   }
   return status;
}

utilStatus collSet_clear( collSet self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   collSetImpl * This = collSet_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      This->count = 0;
#ifndef STATIC_ALLOCATION
      This->limit = 100;
      This->items = (collSetItem *)realloc( This->items, This->limit * sizeof( collSetItem ));
#endif
   }
   return status;
}

utilStatus collSet_add( collSet self, collSetItem item ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   collSetImpl * This = collSet_safeCast( self, &status );
   collSetItem * prev;
   if( status != UTIL_STATUS_NO_ERROR ) {
      return status;
   }
   prev =
      bsearch(
         &item,
         This->items,
         This->count,
         sizeof( collSetItem ),
         (PVoidComparator)This->cmp );
   if( prev ) {
      return UTIL_STATUS_DUPLICATE;
   }
#ifdef STATIC_ALLOCATION
   if( This->count == collSet_ITEM_MAX_COUNT ) {
      return UTIL_STATUS_TOO_MANY;
   }
#else
   if( This->count == This->limit ) {
      This->limit += 100;
      This->items = (collSetItem *)realloc( This->items, This->limit * sizeof( collSetItem ));
      if( This->items == NULL ) {
         This->limit = 0;
         This->count = 0;
         return UTIL_STATUS_TOO_MANY;
      }
   }
#endif
   This->items[This->count++] = item;
   qsort( This->items, This->count, sizeof( collSetItem ), (PVoidComparator)This->cmp );
   return UTIL_STATUS_NO_ERROR;
}

utilStatus collSet_remove( collSet self, collSetItem item ) {
   collSetItem * prev;
   unsigned      moved;
   utilStatus    status = UTIL_STATUS_NO_ERROR;
   collSetImpl * This   = collSet_safeCast( self, &status );
   if( status != UTIL_STATUS_NO_ERROR ) {
      return status;
   }
   prev =
      bsearch(
         &item,
         This->items,
         This->count,
         sizeof( collSetItem ),
         (PVoidComparator)This->cmp );
   if( prev == NULL ) {
      return UTIL_STATUS_NOT_FOUND;
   }
   moved = This->items + This->count - 1 - prev;
   memmove( prev, prev + 1, moved*sizeof( collSetItem ));
   --This->count;
   return UTIL_STATUS_NO_ERROR;
}

utilStatus collSet_size( collSet self, unsigned int * result ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( result == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      collSetImpl * This = collSet_safeCast( self, &status );
      if( status == UTIL_STATUS_NO_ERROR ) {
         *result = This->count;
      }
      else {
         *result = 0U;
      }
   }
   return status;
}

utilStatus collSet_contains( collSet self, collSetItem item, bool * result ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( result == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      collSetImpl * This = collSet_safeCast( self, &status );
      *result = false;
      if( status == UTIL_STATUS_NO_ERROR ) {
         collSetItem * prev =
            bsearch(
               &item,
               This->items,
               This->count,
               sizeof( collSetItem ),
               (PVoidComparator)This->cmp );
         *result = ( prev == NULL );
      }
   }
   return status;
}

utilStatus collSet_foreach( collSet self, collForeachFunction fn, void * userData ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   collSetImpl * This = collSet_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      collForeach context;
      memset( &context, 0, sizeof( context ));
      context.user = userData;
      for( context.index = 0;
           ( UTIL_STATUS_NO_ERROR == status ) && ( context.index < This->count );
           ++context.index )
      {
         context.value = This->items[context.index];
         status = fn( &context );
      }
   }
   return status;
}

utilStatus collSet_values( collSet self, collSetValues * result ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( result == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      collSetImpl * This = collSet_safeCast( self, &status );
      if( status == UTIL_STATUS_NO_ERROR ) {
         *result = This->items;
      }
   }
   return status;
}
