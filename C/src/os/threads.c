#include <os/threads.h>

bool osThread_create( osThread * thread, osThreadRoutine entry, void * arg ) {
#ifdef WIN32
   return TRUE == CreateThread( NULL, 0, entry, arg, 0, thread );
#else
   return 0 == pthread_create( thread, NULL, entry, arg );
#endif
}

bool osThread_join( osThread thread ) {
#ifdef WIN32
   return WAIT_OBJECT_0 == WaitForSingleObject( thread, INFINITE );
#else
   return 0 == pthread_join( thread, NULL );
#endif
}
