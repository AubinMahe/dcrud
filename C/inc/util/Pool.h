#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <util/Status.h>

#include <string.h>
#include <stdio.h>

typedef struct utilPoolEntry_s {

   void *   item;
   unsigned refCount;

} utilPoolEntry;

typedef struct utilPool_s {

   utilPoolEntry * entries;
   unsigned        entriesCount;

} utilPool;

utilStatus utilPool_dumpAll       ( FILE * target );
utilStatus utilPool_new           ( utilPool * This, const char *    name,
                                                     void *          items,
                                                     unsigned        itemSize,
                                                     utilPoolEntry * entries,
                                                     unsigned        entriesCount );
utilStatus utilPool_reserve       ( utilPool * This, void *          newEntry );
utilStatus utilPool_addReferenceTo( utilPool * This, const void *    entry );
utilStatus utilPool_release       ( utilPool * This, void *          entry );

#ifdef STATIC_ALLOCATION

#  define UTIL_POOL_DECLARE(T)\
   bool          T ## PoolInit = true;\
   unsigned      T ## PoolSize =   T ## _POOL_SIZE;\
   T             T ## PoolData   [ T ## _POOL_SIZE ];\
   utilPoolEntry T ## PoolEntries[ T ## _POOL_SIZE ];\
   utilPool      T ## Pool;

#  define UTIL_POOL_INIT(T)\
   if( T ## PoolInit ) {\
      status = utilPool_new(\
         &T ## Pool, #T,\
         T ## PoolData, sizeof( T ), T ## PoolEntries, T ## PoolSize );\
      if( UTIL_STATUS_NO_ERROR == status ) {\
         T ## PoolInit = false;\
      }\
   }

#  define UTIL_ALLOCATE(T)\
   UTIL_POOL_INIT(T)\
   status = utilPool_reserve( &T ## Pool, &This );\
   if( UTIL_STATUS_NO_ERROR == status ) {\
      memset( This, 0, sizeof( T ));\
   }\
   *self = (T *)This

#  define UTIL_ALLOCATE_ADT(T,A,C)\
   UTIL_POOL_INIT(T ## Impl)\
   status = utilPool_reserve( &T ## ImplPool, &C );\
   if( UTIL_STATUS_NO_ERROR == status ) {\
      memset( C, 0, sizeof( T ## Impl ));\
      C->magic = T ## ImplMAGIC;\
   }\
   *A = (T)C

#  define UTIL_RELEASE(T) status = utilPool_release( &T ## Pool, self );

#else

#  define UTIL_POOL_DECLARE(T)

#  define UTIL_POOL_INIT(T)

#  define UTIL_ALLOCATE(T)\
   This = (T *)malloc( sizeof( T ));\
   if( NULL == This ) {\
      status = UTIL_STATUS_TOO_MANY;\
   }\
   else {\
      memset( This, 0, sizeof( T ));\
   }\
   *self = This

#  define UTIL_ALLOCATE_ADT(T,A,C)\
   C = (T ## Impl *)malloc( sizeof( T ## Impl ));\
   if( NULL == C ) {\
      status = UTIL_STATUS_TOO_MANY;\
   }\
   else {\
      memset( C, 0, sizeof( T ## Impl ));\
      C->magic = T ## ImplMAGIC;\
      status = UTIL_STATUS_NO_ERROR;\
   }\
   *A = (T)C

#  define UTIL_RELEASE(T)\
   if( self ) {\
      free( *self );\
      *self = NULL;\
      status = UTIL_STATUS_NO_ERROR;\
   }\
   else {\
      status = UTIL_STATUS_NULL_ARGUMENT;\
   }

#endif

#ifdef __cplusplus
}
#endif
