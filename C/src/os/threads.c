#include <os/threads.h>

bool osCreateThread( osThreadRoutine entry, osThread * thread, void * arg ) {
#ifdef WIN32
   return TRUE == CreateThread( NULL, 0, entry, arg, 0, thread );
#else
   return 0 == pthread_create( thread, NULL, entry, arg );
#endif
}
