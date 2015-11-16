#include <coll/Map.h>
#include <coll/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct collMapPair_s {

   collMapKey   key;
   collMapValue value;

} collMapPair;

typedef struct collPrivateMap_s {

   collComparator cmp;
   unsigned       count;
#ifdef STATIC_ALLOCATION
   collMapPair    pairs[COLL_MAP_ITEM_MAX_COUNT];
#else
   unsigned       limit;
   collMapPair *  pairs;
#endif

} collPrivateMap;

#ifdef STATIC_ALLOCATION
static collPrivateMap Maps[COLL_MAP_MAX_COUNT];
static unsigned int   NextMap = 0;
unsigned int collLimitsMapCountMax     = 0;
unsigned int collLimitsMapPairCountMax = 0;
#endif

typedef int ( * PVoidComparator )( const void *, const void * );

collMap collMap_reserve( collComparator cmp ) {
#ifdef STATIC_ALLOCATION
   if( NextMap == COLL_MAP_MAX_COUNT ) {
      fprintf( stderr, "%s:%d:collMap_reserve: out of memory!\n", __FILE__, __LINE__ );
      return NULL;
   }
   collPrivateMap * This = &Maps[NextMap++];
#else
   collPrivateMap * This = (collPrivateMap *)malloc( sizeof( collPrivateMap ));
#endif
   This->cmp   = cmp;
   This->count = 0;
#ifndef STATIC_ALLOCATION
   This->limit = 100;
   This->pairs = (collMapPair *)malloc( This->limit * sizeof( collMapPair ));
#endif
   return (collMap)This;
}

void collMap_clear( collMap self ) {
   collPrivateMap * This = (collPrivateMap *)self;
   This->count = 0;
#ifndef STATIC_ALLOCATION
   This->limit = 100;
   This->pairs = (collMapPair *)realloc( This->pairs, This->limit * sizeof( collMapPair ) );
#endif
}

collMapValue collMap_put( collMap self, collMapKey key, collMapValue value ) {
   collPrivateMap * This = (collPrivateMap *)self;
   collMapPair *    pair;
   collMapValue     oldValue = NULL;
   collMapPair      keyPair;

   keyPair.key   = key;
   keyPair.value = NULL;
   pair =
      bsearch(
         &keyPair, This->pairs, This->count, sizeof( collMapPair ), (PVoidComparator)This->cmp );
   if( pair ) {
      oldValue = pair->value;
      pair->value = value;
   }
   else {
#ifdef STATIC_ALLOCATION
   if( This->count == COLL_MAP_ITEM_MAX_COUNT ) {
      fprintf( stderr, "%s:%d:collMap_put: out of memory!\n", __FILE__, __LINE__ );
      return NULL;
   }
#else
   if( This->count == This->limit ) {
      This->limit += 100;
      This->pairs = (collMapPair *)realloc( This->pairs, This->limit * sizeof( collMapPair ));
   }
#endif
      pair = (collMapPair *)(This->pairs + This->count++);
      pair->key   = key;
      pair->value = value;
      qsort( This->pairs, This->count, sizeof( collMapPair ), (PVoidComparator)This->cmp );
   }
   return oldValue;
}

collMapValue collMap_get( collMap self, collMapKey key ) {
   collPrivateMap * This = (collPrivateMap *)self;
   collMapPair *    pair;
   collMapPair      keyPair;

   keyPair.key   = key;
   keyPair.value = NULL;
   pair =
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
   unsigned         i;
   collMapPair      keyPair;

   keyPair.key   = key;
   keyPair.value = NULL;
   for( i = 0; i < This->count; ++i ) {
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
   This->count = 0;
#ifndef STATIC_ALLOCATION
   free( This->pairs );
   This->limit = 0;
   This->pairs = NULL;
   free( This );
#endif
   *self = NULL;
}
