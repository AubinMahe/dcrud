#include <coll/Set.h>
#include <stdlib.h>
#include <string.h>

typedef struct collPrivateSet_s {

   unsigned       count;
   unsigned       limit;
   collSetItem *  items;
   collComparator cmp;

} collPrivateSet;

typedef int ( * PVoidComparator )( const void *, const void * );

collSet collSet_reserve( collComparator cmp ) {
   collPrivateSet * This = (collPrivateSet *)malloc( sizeof( collPrivateSet ));
   This->count = 0;
   This->limit = 100;
   This->items = (collSetItem *)malloc( This->limit * sizeof( collSetItem ));
   This->cmp   = cmp;
   return (collSet)This;
}

void collSet_clear( collSet self ) {
   collPrivateSet * This = (collPrivateSet *)self;
   This->count = 0;
   This->limit = 100;
   This->items = (collSetItem *)realloc( This->items, This->limit * sizeof( collSetItem ));
}

bool collSet_add( collSet self, collSetItem item ) {
   collPrivateSet * This = (collPrivateSet *)self;
   collSetItem prev =
      bsearch( &item, This->items, This->count, sizeof( collSetItem ), (PVoidComparator)This->cmp );
   if( prev ) {
      return false;
   }
   if( This->count == This->limit ) {
      This->limit += 100;
      This->items = (collSetItem *)realloc( This->items, This->limit * sizeof( collSetItem ));
   }
   This->items[This->count++] = item;
   qsort( This->items, This->count, sizeof( collSetItem ), (PVoidComparator)This->cmp );
   return true;
}

bool collSet_remove( collSet self, collSetItem item ) {
   collPrivateSet * This = (collPrivateSet *)self;
   for( unsigned i = 0; i < This->count; ++i ) {
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
   free( This->items );
   This->count = 0;
   This->limit = 0;
   This->items = NULL;
   free( This );
   *self = NULL;
}
