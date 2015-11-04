#include <coll/Map.h>
#include <stdlib.h>
#include <string.h>

typedef struct collMapPair_s {

   collMapKey   key;
   collMapValue value;

} collMapPair;

typedef struct collPrivateMap_s {

   unsigned       count;
   unsigned       limit;
   collComparator cmp;
   collMapPair *  pairs;

} collPrivateMap;

typedef int ( * PVoidComparator )( const void *, const void * );

collMap collMap_reserve( collComparator cmp ) {
   collPrivateMap * This = (collPrivateMap *)malloc( sizeof( collPrivateMap ));
   This->count = 0;
   This->limit = 100;
   This->pairs = (collMapPair *)malloc( This->limit * sizeof( collMapPair ));
   This->cmp   = cmp;
   return (collMap)This;
}

void collMap_clear( collMap self ) {
   collPrivateMap * This = (collPrivateMap *)self;
   This->count = 0;
   This->limit = 100;
   This->pairs = (collMapPair *)realloc( This->pairs, This->limit * sizeof( collMapPair ) );
}

collMapValue collMap_put( collMap self, const collMapKey key, collMapValue value ) {
   collPrivateMap * This    = (collPrivateMap *)self;
   collMapPair      keyPair = { key, NULL };
   collMapPair *    pair    =
      bsearch(
         &keyPair, This->pairs, This->count, sizeof( collMapPair ), (PVoidComparator)This->cmp );
   collMapValue oldValue = NULL;
   if( pair ) {
      oldValue = pair->value;
      pair->value = value;
   }
   else {
      if( This->count == This->limit ) {
         This->limit += 100;
         This->pairs = (collMapPair *)realloc( This->pairs, This->limit * sizeof( collMapPair ));
      }
      pair = (collMapPair *)(This->pairs + This->count++);
      pair->key   = key;
      pair->value = value;
      qsort( This->pairs, This->count, sizeof( collMapPair ), (PVoidComparator)This->cmp );
   }
   return oldValue;
}

collMapValue collMap_get( collMap self, const collMapKey key ) {
   collPrivateMap * This    = (collPrivateMap *)self;
   collMapPair      keyPair = { key, NULL };
   collMapPair *    pair    =
      bsearch(
         &keyPair,
         This->pairs,
         This->count,
         sizeof( collMapPair ),
         (PVoidComparator)This->cmp );
   if( pair ) {
      return pair->value;
   }
   return 0;
}

collMapValue collMap_remove( collMap self, collMapKey key ) {
   collPrivateMap * This = (collPrivateMap *)self;
   collMapPair      keyPair = { key, NULL };
   for( unsigned i = 0; i < This->count; ++i ) {
      if( 0 == This->cmp( This->pairs + i, &keyPair )) {
         collMapValue retVal = This->pairs[i].value;
         memmove(
            This->pairs + i,
            This->pairs + i + 1,
            ( This->count - i - 1 )*sizeof( collMapPair ));
         --This->count;
         return retVal;
      }
   }
   return NULL;
}

unsigned int collMap_size( collMap self ) {
   return ((collPrivateMap *)self)->count;
}

collForeachMapResult collMap_foreach( collMap self, collForeachMapFunction fn, void * userData ) {
   collPrivateMap * This = (collPrivateMap *)self;
   collForeachMap   context;
   context.user = userData;
   for( context.index = 0; context.index < This->count; ++context.index ) {
      context.key   = This->pairs[context.index].key;
      context.value = This->pairs[context.index].value;
      if( !fn( &context ) ) {
         break;
      }
   }
   return context.retVal;
}

void collMap_release( collMap * self ) {
   collPrivateMap * This = (collPrivateMap *)*self;
   free( This->pairs );
   This->count = 0;
   This->limit = 0;
   This->pairs = NULL;
   free( This );
   *self = NULL;
}
