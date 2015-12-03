#include <os/Mutex.h>
#ifdef WIN32
#  include <windows.h>
#else
#  include <pthread.h>
#endif

typedef struct osMutexPrivate_s {

#ifdef WIN32
   HANDLE          mutex;
#else
   pthread_mutex_t mutex;
#endif

} osMutexPrivate;

osStatus osMutex_new( osMutex * target ) {
   osStatus            retval = OS_STATUS_NO_ERROR;
   osMutexPrivate *    This   = (osMutexPrivate *)malloc( sizeof( osMutexPrivate ));
#ifdef WIN32
   This->mutex = CreateMutex( NULL, FALSE, NULL );
#else
   pthread_mutexattr_t attr;
   pthread_mutexattr_init( &attr);
   pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE );
   retval  = (osStatus)pthread_mutex_init( &This->mutex, &attr );
#endif
   *target = (osMutex)This;
   return retval;
}

osStatus osMutex_delete( osMutex * self ) {
   osMutexPrivate * This = (osMutexPrivate *)*self;
   osStatus retval = OS_STATUS_NO_ERROR;
   if( This ) {
#ifdef WIN32
      CloseHandle( &This->mutex );
      retval = OS_STATUS_NO_ERROR;
#else
      retval = (osStatus)pthread_mutex_destroy( &This->mutex );
#endif
      free( This );
      *self = NULL;
   }
   return retval;
}

osStatus osMutex_take( osMutex self ) {
   osMutexPrivate * This = (osMutexPrivate *)self;
#ifdef WIN32
   return (osStatus)WaitForSingleObject( &This->mutex, INFINITE );
#else
   return (osStatus)pthread_mutex_lock( &This->mutex );
#endif
}

osStatus osMutex_release( osMutex self ) {
   osMutexPrivate * This = (osMutexPrivate *)self;
#ifdef WIN32
   return ReleaseMutex( &This->mutex ) ? OS_STATUS_NO_ERROR : (osStatus)GetLastError();
#else
   return (osStatus)pthread_mutex_unlock( &This->mutex );
#endif
}
