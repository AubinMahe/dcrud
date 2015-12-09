#include <os/Mutex.h>
#if defined( WIN32 ) || defined( _WIN32 )
#  include <windows.h>
#else
#  include <pthread.h>
#endif

typedef struct osMutexPrivate_s {

#if defined( WIN32 )
   HANDLE          mutex;
#else
   pthread_mutex_t mutex;
#endif

} osMutexPrivate;

int osMutex_new( osMutex * target ) {
   int            retval = 0;
   osMutexPrivate *    This   = (osMutexPrivate *)malloc( sizeof( osMutexPrivate ));
#ifdef WIN32
   This->mutex = CreateMutex( NULL, FALSE, NULL );
#else
   pthread_mutexattr_t attr;
   pthread_mutexattr_init( &attr);
   pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE );
   retval  = (int)pthread_mutex_init( &This->mutex, &attr );
#endif
   *target = (osMutex)This;
   return retval;
}

int osMutex_delete( osMutex * self ) {
   osMutexPrivate * This = (osMutexPrivate *)*self;
   int retval = 0;
   if( This ) {
#ifdef WIN32
      CloseHandle( &This->mutex );
      retval = 0;
#else
      retval = (int)pthread_mutex_destroy( &This->mutex );
#endif
      free( This );
      *self = NULL;
   }
   return retval;
}

int osMutex_take( osMutex self ) {
   osMutexPrivate * This = (osMutexPrivate *)self;
#ifdef WIN32
   return (int)WaitForSingleObject( &This->mutex, INFINITE );
#else
   return (int)pthread_mutex_lock( &This->mutex );
#endif
}

int osMutex_release( osMutex self ) {
   osMutexPrivate * This = (osMutexPrivate *)self;
#ifdef WIN32
   return ReleaseMutex( &This->mutex ) ? 0 : (int)GetLastError();
#else
   return (int)pthread_mutex_unlock( &This->mutex );
#endif
}
