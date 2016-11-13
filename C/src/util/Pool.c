#include <util/Pool.h>
#include <coll/Map.h>

#include <stdlib.h>

typedef struct PoolMapEntry_s {

   const char * name;
   utilPool *   pool;
   unsigned     reserveCount;
   unsigned     releaseCount;
   unsigned     reservePeak;

} PoolMapEntry;

static PoolMapEntry poolMap[9999];
static unsigned     poolMapCount;

utilStatus utilPool_dumpAll( FILE * target ) {
#ifdef STATIC_ALLOCATION
   unsigned i;

   if( NULL == target ) {
      target = stderr;
   }
   fprintf( target,
      "+------------------------------+----------+----------+----------+----------+--------------+\n"
      "|             Pool             | Capacity | Reserved | Reserve# | Release# | Max Reserved |\n"
      "+------------------------------+----------+----------+----------+----------+--------------+\n" );
   for( i = 0U; i < poolMapCount; ++i ) {
      PoolMapEntry *  entry        = poolMap + i;
      const char *    name         = entry->name;
      utilPool *      pool         = entry->pool;
      utilPoolEntry * entries      = pool->entries;
      unsigned        entriesCount = pool->entriesCount;
      unsigned j, count = 0U;
      for( j = 0U; j < entriesCount; ++j ) {
         if( entries[j].refCount > 0 ) {
            ++count;
         }
      }
      fprintf( target, "|%-30s|%9d |%9d |%9d |%9d |%13d |\n",
         name, entriesCount, count,
         entry->reserveCount, entry->releaseCount, entry->reservePeak );
   }
   fprintf( target,
      "+------------------------------+----------+----------+----------+----------+--------------+\n" );
#else
   (void)target;
#endif
   return UTIL_STATUS_NO_ERROR;
}

utilStatus utilPool_new(
   utilPool *      This,
   const char *    name,
   void *          items,
   unsigned        itemSize,
   utilPoolEntry * poolEntries,
   unsigned        entriesCount )
{
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( This == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      unsigned i;
      This->entries      = poolEntries;
      This->entriesCount = entriesCount;
      for( i = 0; i < entriesCount; ++i ) {
         poolEntries[i].refCount = 0U;
         poolEntries[i].item     = (char *)items + itemSize*i;
      }
      poolMap[poolMapCount].name = name;
      poolMap[poolMapCount].pool = This;
      ++poolMapCount;
   }
   return status;
}

static void incrementUseOf( const utilPool * This ) {
   unsigned i;
   for( i = 0U; i < poolMapCount; ++i ) {
      PoolMapEntry * entry = poolMap + i;
      if( entry->pool == This ) {
         unsigned delta = (++(entry->reserveCount)) - entry->releaseCount;
         if( delta > entry->reservePeak ) {
            entry->reservePeak = delta;
         }
         return;
      }
   }
}

static void decrementUseOf( const utilPool * This ) {
   unsigned i;
   for( i = 0U; i < poolMapCount; ++i ) {
      PoolMapEntry * entry = poolMap + i;
      if( entry->pool == This ) {
         entry->releaseCount++;
         return;
      }
   }
}

utilStatus utilPool_reserve( utilPool * This, void * arg ) {
   void ** newEntry = (void**)arg;
   utilStatus status = UTIL_STATUS_TOO_MANY;
   if( This == NULL || newEntry == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      unsigned i = 0;
      *newEntry = NULL;
      while(( i < This->entriesCount ) && ( UTIL_STATUS_TOO_MANY == status )){
         if( This->entries[i].refCount == 0 ) {
            utilPoolEntry * cell = This->entries+i;
            (cell->refCount)++;
            *newEntry = cell->item;
            status = UTIL_STATUS_NO_ERROR;
            incrementUseOf( This );
         }
         else {
            ++i;
         }
      }
   }
   return status;
}

static utilPoolEntry * getEntry( utilPool * This, const void * item ) {
   unsigned i = 0U;
   for( i = 0U; i < This->entriesCount; ++i ) {
      utilPoolEntry * entry = This->entries+i;
      if( entry->item == item ) {
         return entry;
      }
   }
   return NULL;
}

utilStatus utilPool_addReferenceTo( utilPool * This, const void * target ) {
   utilStatus status = UTIL_STATUS_NOT_FOUND;
   if( This == NULL || target == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      utilPoolEntry * entry = getEntry( This, target );
      if( entry ) {
         entry->refCount = entry->refCount + 1;
         status = UTIL_STATUS_NO_ERROR;
      }
   }
   return status;
}

utilStatus utilPool_release( utilPool * This, void * arg ) {
   void **    target = arg;
   utilStatus status = UTIL_STATUS_NOT_FOUND;
   if( This == NULL || target == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      utilPoolEntry * entry = getEntry( This, *target );
      if( NULL == entry ) {
         status = UTIL_STATUS_NOT_FOUND;
      }
      else if( entry->refCount > 0 ) {
         entry->refCount = entry->refCount - 1;
         if( 0 == entry->refCount ) {
            *target = NULL;
            decrementUseOf( This );
         }
         status = UTIL_STATUS_NO_ERROR;
      }
      else {
         status = UTIL_STATUS_DUPLICATE;
      }
   }
   return status;
}
