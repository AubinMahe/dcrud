#include <coll/List.h>
#include "magic.h"
#include "poolSizes.h"

#include <util/Pool.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct collListImpl_s {

   unsigned       magic;
#ifdef STATIC_ALLOCATION
   collListItem   items[collList_ITEM_MAX_COUNT];
#else
   unsigned       limit;
   collListItem * items;
#endif
   unsigned       count;

} collListImpl;

UTIL_DEFINE_SAFE_CAST( collList     )
UTIL_POOL_DECLARE    ( collListImpl )

utilStatus collList_new( collList * self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( NULL == self ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      collListImpl * This = NULL;
      UTIL_ALLOCATE_ADT( collList, self, This  );
#ifndef STATIC_ALLOCATION
      if( status == UTIL_STATUS_NO_ERROR ) {
         This->limit = 100;
         This->items = (collListItem *)malloc( This->limit * sizeof( collListItem ));
         if( This->items == NULL ) {
            free( This );
            status = UTIL_STATUS_TOO_MANY;
            *self  = NULL;
         }
      }
#endif
   }
   return status;
}

utilStatus collList_delete( collList * self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( NULL == self ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      collListImpl * This = collList_safeCast( *self, &status );
      if( UTIL_STATUS_NO_ERROR == status ) {
         (void)This;
#ifndef STATIC_ALLOCATION
         free( This->items );
#endif
         UTIL_RELEASE( collListImpl );
         *self  = NULL;
      }
   }
   return status;
}

utilStatus collList_clear( collList self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   collListImpl * This = collList_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      This->count = 0;
#ifndef STATIC_ALLOCATION
      This->limit = 100;
      This->items = (collListItem *)realloc( This->items, This->limit * sizeof( collListItem ));
#endif
   }
   return status;
}

utilStatus collList_add( collList self, collListItem item ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   collListImpl * This = collList_safeCast( self, &status );
   if( status != UTIL_STATUS_NO_ERROR ) {
      return status;
   }
#ifdef STATIC_ALLOCATION
   if( This->count == collList_ITEM_MAX_COUNT ) {
      return UTIL_STATUS_TOO_MANY;
   }
#else
   if( This->count == This->limit ) {
      This->limit += 100;
      This->items = (collListItem *)realloc( This->items, This->limit * sizeof( collListItem ));
      if( This->items == NULL ) {
         This->count = 0;
         This->limit = 0;
         return UTIL_STATUS_TOO_MANY;
      }
   }
#endif
   This->items[This->count++] = item;
   return UTIL_STATUS_NO_ERROR;
}

utilStatus collList_remove( collList self, collListItem item ) {
   utilStatus     status = UTIL_STATUS_NO_ERROR;
   collListImpl * This   = collList_safeCast( self, &status );
   unsigned       i      = 0;
   if( status != UTIL_STATUS_NO_ERROR ) {
      return status;
   }
   status = UTIL_STATUS_NOT_FOUND;
   while((status == UTIL_STATUS_NOT_FOUND) && (i < This->count)) {
      if( This->items[i] == item ) {
         memmove(
            This->items + i,
            This->items + i + 1,
            ( This->count - i - 1 )*sizeof( collListItem ));
         --This->count;
         status = UTIL_STATUS_NO_ERROR;
      }
      else {
         ++i;
      }
   }
   return status;
}

utilStatus collList_get( collList self, unsigned int index, collListItem * result ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( result == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      collListImpl * This = collList_safeCast( self, &status );
      if( status == UTIL_STATUS_NO_ERROR ) {
         if( index < This->count ) {
            *result = This->items[index];
         }
         else {
            *result = NULL;
            status = UTIL_STATUS_OUT_OF_RANGE;
         }
      }
   }
   return status;
}

utilStatus collList_size( collList self, unsigned int * result ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( result == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      collListImpl * This = collList_safeCast( self, &status );
      if( status == UTIL_STATUS_NO_ERROR ) {
         *result = This->count;
      }
      else {
         *result = 0U;
      }
   }
   return status;
}

utilStatus collList_indexOf(
   collList       self,
   collListItem   item,
   collComparator cmp,
   unsigned *     result )
{
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( result == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      collListImpl * This = collList_safeCast( self, &status );
      if( status == UTIL_STATUS_NO_ERROR ) {
         unsigned i = 0U;
         status = UTIL_STATUS_NOT_FOUND;
         while(( i < This->count ) && ( status == UTIL_STATUS_NOT_FOUND )) {
            if( 0 == cmp( &(This->items[i]), &item )) {
               status  = UTIL_STATUS_NO_ERROR;
               *result = i;
            }
         }
      }
   }
   return status;
}

utilStatus collList_contains(
   collList       self,
   collListItem   item,
   collComparator cmp,
   bool *         result )
{
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( result == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      unsigned index = 0;
      status = collList_indexOf( self, item, cmp, &index );
      *result = ( status == UTIL_STATUS_NO_ERROR );
   }
   return status;
}

utilStatus collList_foreach(
   collList            self,
   collForeachFunction fn,
   void *              userData,
   collForeachResult * result   )
{
   utilStatus status = UTIL_STATUS_NO_ERROR;
   collListImpl * This = collList_safeCast( self, &status );
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
      if( result ) {
         *result = context.retVal;
      }
   }
   return status;
}

utilStatus collList_values( collList self, collListValues * result ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( result == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      collListImpl * This = collList_safeCast( self, &status );
      if( status == UTIL_STATUS_NO_ERROR ) {
         *result = This->items;
      }
   }
   return status;
}
