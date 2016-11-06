#include <channel/Factories.h>
#include "poolSizes.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Entry_s {

   int              classID;
   channelFactory * factory;

} channelFactoryImpl;

#ifdef STATIC_ALLOCATION
static channelFactoryImpl   s_factories[channelFactoryMaxCount];
static unsigned             s_capacity = channelFactoryMaxCount;
#else
static channelFactoryImpl * s_factories;
static unsigned             s_capacity = 0U;
#endif
static unsigned             s_count    = 0U;

int FactoryComparator( const void * l, const void * r ) {
   const channelFactoryImpl * left  = (const channelFactoryImpl *)l;
   const channelFactoryImpl * right = (const channelFactoryImpl *)r;
   return left->classID - right->classID;
}

utilStatus channelFactories_add( channelFactory * factory ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( factory == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      const int classID = factory->getClassID();
      channelFactoryImpl * entry;
      if( s_count == s_capacity ) {
#ifdef STATIC_ALLOCATION
         status = UTIL_STATUS_TOO_MANY;
#else
         channelFactoryImpl * ra = (channelFactoryImpl *)
            realloc( s_factories, ( s_capacity + 100 )*sizeof( channelFactoryImpl ));
         if( ra == NULL ) {
            status = UTIL_STATUS_TOO_MANY;
         }
         else {
            s_capacity += 100;
            s_factories = ra;
         }
#endif
      }
      if( UTIL_STATUS_NO_ERROR == status ) {
         entry = &(s_factories[s_count++]);
         entry->classID = classID;
         entry->factory = factory;
         qsort( s_factories, s_count, sizeof( channelFactoryImpl ), FactoryComparator );
      }
   }
   return status;
}

utilStatus channelFactories_get( int classID, channelFactory ** result ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( result == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      channelFactoryImpl   key;
      channelFactoryImpl * entry;
      memset( &key, 0, sizeof( key ));
      key.classID = classID;
      entry =
         bsearch( &key, s_factories, s_count, sizeof( channelFactoryImpl ), FactoryComparator );
      if( entry == NULL ) {
         status = UTIL_STATUS_NOT_FOUND;
      }
      else {
         *result = entry->factory;
      }
   }
   return status;
}

utilStatus channelFactories_done() {
   utilStatus status = UTIL_STATUS_NO_ERROR;
#ifndef STATIC_ALLOCATION
   free( s_factories );
   s_factories = NULL;
#endif
   s_count = 0U;
   return status;
}
