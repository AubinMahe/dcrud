#include <os/threads.h>

bool osThread_create( osThread * thread, osThreadRoutine entry, void * arg ) {
#ifdef WIN32
   DWORD threadId = 0;
   if( thread ) {
      *thread = CreateThread( NULL, 0,(LPTHREAD_START_ROUTINE)entry, arg, 0, &threadId );
      return *thread != NULL;
   }
   return false;
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
