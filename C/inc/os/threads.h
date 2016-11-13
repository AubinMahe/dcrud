#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <util/types.h>

#ifdef WIN32
#  ifndef STRICT
#     define STRICT 1
#  endif
#  include <windows.h>
   typedef HANDLE                   osThread;
   typedef LPTHREAD_START_ROUTINE   osThreadRoutine;
#else
#  include <pthread.h>
   typedef pthread_t                osThread;
   typedef void *( *                osThreadRoutine )( void * );
#endif

bool osThread_create( osThread * thread, osThreadRoutine entry, void * arg );
bool osThread_join  ( osThread   thread );

#ifdef __cplusplus
}
#endif
