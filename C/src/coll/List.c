#include <coll/List.h>
#include <stdlib.h>
#include <string.h>

typedef struct collPrivateList_s {

   unsigned       count;
   unsigned       limit;
   collListItem * items;

} collPrivateList;

collList collList_reserve() {
   collPrivateList * This = (collPrivateList *)malloc( sizeof( collPrivateList ));
   This->count = 0;
   This->limit = 100;
   This->items = (collListItem *)malloc( This->limit * sizeof( collListItem ));
   return (collList)This;
}

void collList_clear( collList self ) {
   collPrivateList * This = (collPrivateList *)self;
   This->count = 0;
   This->limit = 100;
   This->items = (collListItem *)realloc( This->items, This->limit * sizeof( collListItem ));
}

void collList_add( collList self, collListItem item ) {
   collPrivateList * This = (collPrivateList *)self;
   if( This->count == This->limit ) {
      This->limit += 100;
      This->items = (collListItem *)realloc( This->items, This->limit * sizeof( collListItem ) );
   }
   This->items[This->count++] = item;
}

bool collList_remove( collList self, collListItem item ) {
   collPrivateList * This = (collPrivateList *)self;
   for( unsigned i = 0; i < This->count; ++i ) {
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

unsigned int collList_size( collList self ) {
   return ((collPrivateList *)self )->count;
}

collForeachResult collList_foreach( collList self, collForeachFunction fn, void * userData ) {
   collPrivateList * This = (collPrivateList *)self;
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

void collList_release( collList * self ) {
   collPrivateList * This = (collPrivateList *)*self;
   free( This->items );
   This->count = 0;
   This->limit = 0;
   This->items = NULL;
   free( This );
   *self = NULL;
}
