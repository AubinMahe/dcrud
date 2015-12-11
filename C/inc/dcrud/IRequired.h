#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <dcrud/Status.h>
#include <dcrud/ICallback.h>

typedef enum dcrudQueueIndex_e {

   DCRUD_VERY_URGENT_QUEUE =   0,
   DCRUD_URGENT_QUEUE      =  50,
   DCRUD_DEFAULT_QUEUE     = 100,
   DCRUD_NON_URGENT_QUEUE  = 255

} dcrudQueueIndex;

typedef enum dcrudCallMode_e {

   DCRUD_SYNCHRONOUS,
   DCRUD_ASYNCHRONOUS_DEFERRED,
   DCRUD_ASYNCHRONOUS_IMMEDIATE

} dcrudCallMode;

UTIL_ADT( dcrudIRequired );

dcrudStatus dcrudIRequired_call(
   dcrudIRequired This,
   const char *   opName,
   collMap        arguments,
   dcrudICallback callback,
   int *          callId    );

#ifdef __cplusplus
}
#endif
