#pragma once
#ifdef __cplusplus
extern "C" {
#endif

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

utilStatus dcrudIRequired_call(
   dcrudIRequired            This,
   const char *              op,
   struct dcrudArguments_s * args,
   dcrudICallback            callback );

#ifdef __cplusplus
}
#endif
