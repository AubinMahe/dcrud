#include <coll/Set.h>
#include <coll/limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct collPrivateSet_s {

   unsigned       count;
#ifdef STATIC_ALLOCATION
   collSetItem    items[COLL_SET_ITEM_MAX_COUNT];
#else
   unsigned       limit;
   collSetItem *  items;
#endif
   collComparator cmp;

} collPrivateSet;

#ifdef STATIC_ALLOCATION
static collPrivateSet Sets[COLL_SET_MAX_COUNT];
static unsigned int   NextSet = 0;
unsigned int collLimitsSetCountMax     = 0;
unsigned int collLimitsSetItemCountMax = 0;
#endif

typedef int ( * PVoidComparator )( const void *, const void * );

collSet collSet_reserve( collComparator cmp ) {
#ifdef STATIC_ALLOCATION
   if( NextSet == COLL_SET_MAX_COUNT ) {
      fprintf( stderr, "%s:%d:collSet_reserve: out of memory!\n", __FILE__, __LINE__ );
      return NULL;
   }
   collPrivateSet * This = &Sets[NextSet++];
#else
   collPrivateSet * This = (collPrivateSet *)malloc( sizeof( collPrivateSet ));
#endif
   This->count = 0;
#ifndef STATIC_ALLOCATION
   This->limit = 100;
   This->items = (collSetItem *)malloc( This->limit * sizeof( collSetItem ));
#endif
   This->cmp   = cmp;
   return (collSet)This;
}

void collSet_clear( collSet self ) {
   collPrivateSet * This = (collPrivateSet *)self;
   This->count = 0;
#ifndef STATIC_ALLOCATION
   This->limit = 100;
   This->items = (collSetItem *)realloc( This->items, This->limit * sizeof( collSetItem ));
#endif
}

bool collSet_add( collSet self, collSetItem item ) {
   collPrivateSet * This = (collPrivateSet *)self;
   collSetItem prev =
      bsearch( &item, This->items, This->count, sizeof( collSetItem ), (PVoidComparator)This->cmp );
   if( prev ) {
      return false;
   }
#ifdef STATIC_ALLOCATION
   if( This->count == COLL_SET_ITEM_MAX_COUNT ) {
      fprintf( stderr, "%s:%d:collSet_add: out of memory!\n", __FILE__, __LINE__ );
      return false;
   }
#else
   if( This->count == This->limit ) {
      This->limit += 100;
      This->items = (collSetItem *)realloc( This->items, This->limit * sizeof( collSetItem ));
   }
#endif
   This->items[This->count++] = item;
   qsort( This->items, This->count, sizeof( collSetItem ), (PVoidComparator)This->cmp );
   return true;
}

bool collSet_remove( collSet self, collSetItem item ) {
   collPrivateSet * This = (collPrivateSet *)self;
   unsigned         i;
   for( i = 0; i < This->count; ++i ) {
      if( This->items[i] == item ) {
         memmove(
            This->items + i,
            This->items + i + 1,
            ( This->count - i - 1 )*sizeof( collSetItem ));
         --This->count;
         return true;
      }
   }
   return false;
}

unsigned int collSet_size( collSet self ) {
   return ( (collPrivateSet *)self )->count;
}

collForeachResult collSet_foreach( collSet self, collForeachFunction fn, void * userData ) {
   collPrivateSet * This = (collPrivateSet *)self;
   collForeach context;
   memset( &context, 0, sizeof( context ));
   context.user = userData;
   for( context.index = 0; context.index < This->count; ++context.index ) {
      context.item = This->items[context.index];
      if( !fn( &context ) ) {
         break;
      }
   }
   return context.retVal;
}

void collSet_release( collSet * self ) {
   collPrivateSet * This = (collPrivateSet *)*self;
#ifndef STATIC_ALLOCATION
   free( This->items );
#endif
   This->count = 0;
#ifndef STATIC_ALLOCATION
   This->limit = 0;
   This->items = NULL;
   free( This );
#endif
   *self = NULL;
}
