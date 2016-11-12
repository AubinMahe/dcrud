#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <util/types.h>

typedef struct DebugSettings_s {

   bool dumpReceivedBuffer;
   bool dumpNetworkReceiverOperations;

} dcrudDebugSettings_t;

extern dcrudDebugSettings_t * dcrudDebugSettings;

#ifdef __cplusplus
}
#endif
