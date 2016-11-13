#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <util/CmdLine.h>

typedef struct DebugSettings_s {

   bool          dumpReceivedBuffer;
   bool          dumpNetworkReceiverOperations;
   utilStatus (* init )( const utilCmdLine cmdLine );

} utilDebugSettings_t;

extern utilDebugSettings_t * utilDebugSettings;

#ifdef __cplusplus
}
#endif
