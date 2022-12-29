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
#else
#  include <pthread.h>
   typedef pthread_t                osThread;
#endif

typedef void *( * osThreadRoutine )( void * );

bool osThread_create( osThread * thread, osThreadRoutine entry, void * arg );
bool osThread_join  ( osThread   thread );

#ifdef __cplusplus
}
#endif
