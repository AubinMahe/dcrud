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
   typedef DWORD                    utilThread;
   typedef LPTHREAD_START_ROUTINE   utilthreadRoutine;
#else
#  include <pthread.h>
   typedef pthread_t osThread;
   typedef void *( * osThreadRoutine )( void * );
#endif

bool osCreateThread( osThreadRoutine entry, osThread * thread, void * arg );

#ifdef __cplusplus
}
#endif
