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
   osMutexPrivate * This   = (osMutexPrivate *)malloc( sizeof( osMutexPrivate ));
   osStatus         retval = (osStatus)pthread_mutex_init( &This->mutex, NULL );
   *target = (osMutex)This;
   return retval;
}

osStatus osMutex_delete( osMutex * self ) {
   osMutexPrivate * This = (osMutexPrivate *)*self;
   osStatus retval = OS_STATUS_NO_ERROR;
   if( This ) {
      retval = (osStatus)pthread_mutex_destroy( &This->mutex );
      free( This );
      *self = NULL;
   }
   return retval;
}

osStatus osMutex_take( osMutex self ) {
   osMutexPrivate * This = (osMutexPrivate *)self;
   return (osStatus)pthread_mutex_lock( &This->mutex );
}

osStatus osMutex_release( osMutex self ) {
   osMutexPrivate * This = (osMutexPrivate *)self;
   return (osStatus)pthread_mutex_unlock( &This->mutex );
}
