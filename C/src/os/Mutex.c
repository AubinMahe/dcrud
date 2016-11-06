#include <os/Mutex.h>
#include "poolSizes.h"
#include "magic.h"
#include <util/Pool.h>

#if defined( WIN32 ) || defined( _WIN32 )
#  include <windows.h>
#else
#  include <pthread.h>
#endif

typedef struct osMutexImpl_s {

   unsigned        magic;
#if defined( WIN32 )
   HANDLE          mutex;
#else
   pthread_mutex_t mutex;
#endif

} osMutexImpl;

UTIL_DEFINE_SAFE_CAST( osMutex     )
UTIL_POOL_DECLARE    ( osMutexImpl )

utilStatus osMutex_new( osMutex * self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   osMutexImpl * This = NULL;
   UTIL_ALLOCATE_ADT( osMutex, self, This );
   if( status == UTIL_STATUS_NO_ERROR ) {
#ifdef WIN32
      This->mutex = CreateMutex( NULL, FALSE, NULL );
      if( This->mutex == NULL ) {
         status = UTIL_STATUS_STD_API_ERROR;
      }
#else
      pthread_mutexattr_t attr;
      pthread_mutexattr_init( &attr);
      pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE );
      if( pthread_mutex_init( &This->mutex, &attr )) {
         status = UTIL_STATUS_STD_API_ERROR;
      }
#endif
   }
   return status;
}

utilStatus osMutex_delete( osMutex * self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( self == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      osMutexImpl * This = osMutex_safeCast( *self, &status );
      if( status == UTIL_STATUS_NO_ERROR ) {
#ifdef WIN32
         if( ! CloseHandle( &This->mutex )) {
            status = UTIL_STATUS_STD_API_ERROR;
         }
#else
         if( pthread_mutex_destroy( &This->mutex )) {
            status = UTIL_STATUS_STD_API_ERROR;
         }
#endif
         UTIL_RELEASE( osMutexImpl )
      }
   }
   return status;
}

utilStatus osMutex_take( osMutex self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   osMutexImpl * This = osMutex_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
#ifdef WIN32
      DWORD retVal = WaitForSingleObject( &This->mutex, INFINITE );
      if( retVal != WAIT_ABANDONED && retVal != WAIT_OBJECT_0 ) {
         status = UTIL_STATUS_STD_API_ERROR;
      }
#else
      if( pthread_mutex_lock( &This->mutex )) {
         status = UTIL_STATUS_STD_API_ERROR;
      }
#endif
   }
   return status;
}

utilStatus osMutex_release( osMutex self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   osMutexImpl * This = osMutex_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
#ifdef WIN32
      if( ! ReleaseMutex( &This->mutex )) {
         status = UTIL_STATUS_STD_API_ERROR;
      }
#else
      if( pthread_mutex_unlock( &This->mutex )) {
         status = UTIL_STATUS_STD_API_ERROR;
      }
#endif
   }
   return status;
}
