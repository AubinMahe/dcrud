#pragma once
#include <util/types.h>
#include <coll/Comparator.h>

CREATE_ADT( collMap );

typedef void * collMapKey;
typedef void * collMapValue;
typedef void * collForeachMapResult;

typedef struct collForeachMap_s {

   unsigned int         index;
   collMapKey           key;
   collMapValue         value;
   void *               user;
   collForeachMapResult retVal;

} collForeachMap;

typedef bool( *collForeachMapFunction )( collForeachMap * context );

collMap              collMap_reserve( collComparator cmp );
void                 collMap_clear  ( collMap   This );
collMapValue         collMap_put    ( collMap   This, const collMapKey key, collMapValue value );
collMapValue         collMap_remove ( collMap   This, const collMapKey key );
unsigned             collMap_size   ( collMap   This );
collMapValue         collMap_get    ( collMap   This, const collMapKey key );
collForeachMapResult collMap_foreach( collMap   This, collForeachMapFunction fn, void * userData );
void                 collMap_release( collMap * This );