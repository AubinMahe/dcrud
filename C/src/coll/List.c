#include <coll/List.h>
#include <coll/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct List_s {

   unsigned       count;
#ifdef STATIC_ALLOCATION
   collListItem   items[collLIST_ITEM_MAX_COUNT];
#else
   unsigned       limit;
   collListItem * items;
#endif

} List;

#ifdef STATIC_ALLOCATION
static List Lists[collLIST_MAX_COUNT];
static unsigned int    NextList = 0;
unsigned int collLimitsListCountMax     = 0;
unsigned int collLimitsListItemCountMax = 0;
#endif

collList collList_new() {
#ifdef STATIC_ALLOCATION
   if( NextList == collLIST_MAX_COUNT ) {
      fprintf( stderr, "%s:%d:collList_reserve: out of memory!\n", __FILE__, __LINE__ );
      return NULL;
   }
   List * This = &Lists[NextList++];
#else
   List * This = (List *)malloc( sizeof( List ));
#endif
   This->count = 0;
#ifndef STATIC_ALLOCATION
   This->limit = 100;
   This->items = (collListItem *)malloc( This->limit * sizeof( collListItem ));
#endif
   return (collList)This;
}

void collList_delete( collList * self ) {
   List * This = (List *)*self;
   if( This ) {
      This->count = 0;
#ifndef STATIC_ALLOCATION
      free( This->items );
      This->limit = 0;
      This->items = NULL;
      free( This );
#endif
      *self = NULL;
   }
}

void collList_clear( collList self ) {
   List * This = (List *)self;
   This->count = 0;
#ifndef STATIC_ALLOCATION
   This->limit = 100;
   This->items = (collListItem *)realloc( This->items, This->limit * sizeof( collListItem ));
#endif
}

void collList_add( collList self, collListItem item ) {
   List * This = (List *)self;
#ifdef STATIC_ALLOCATION
   if( This->count == collLIST_ITEM_MAX_COUNT ) {
      fprintf( stderr, "%s:%d:collList_add: out of memory!\n", __FILE__, __LINE__ );
      return;
   }
#else
   if( This->count == This->limit ) {
      This->limit += 100;
      This->items = (collListItem *)realloc( This->items, This->limit * sizeof( collListItem ) );
   }
#endif
   This->items[This->count++] = item;
}

bool collList_remove( collList self, collListItem item ) {
   List * This = (List *)self;
   unsigned i;
   for( i = 0; i < This->count; ++i ) {
      if( This->items[i] == item ) {
         memmove(
            This->items + i,
            This->items + i + 1,
            ( This->count - i - 1 )*sizeof( collListItem ));
         --This->count;
         return true;
      }
   }
   return false;
}

collListItem collList_get( collList self, unsigned int index ) {
   List * This = (List *)self;
   return index < This->count ? This->items[index] : NULL;
}

unsigned int collList_size( collList self ) {
   return ((List *)self )->count;
}

collForeachResult collList_foreach( collList self, collForeachFunction fn, void * userData ) {
   List * This = (List *)self;
   collForeach context;
   context.user = userData;
   for( context.index = 0; context.index < This->count; ++context.index ) {
      context.item = This->items[context.index];
      if( !fn( &context ) ) {
         break;
      }
   }
   return context.retVal;
}

collListValues collList_values( collList self ) {
   List * This = (List *)self;
   return This->items;
}
