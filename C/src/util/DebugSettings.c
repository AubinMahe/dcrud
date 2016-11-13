#include <util/DebugSettings.h>

static utilStatus utilDebugSettings_init( const utilCmdLine cmdLine ) {
   utilCmdLine_getBoolean( cmdLine, "dump-received-buffer"            , &utilDebugSettings->dumpReceivedBuffer            );
   utilCmdLine_getBoolean( cmdLine, "dump-network-receiver-operations", &utilDebugSettings->dumpNetworkReceiverOperations );
   return UTIL_STATUS_NO_ERROR;
}

static utilDebugSettings_t theDebugSettings = {
   false, /* bool dumpReceivedBuffer */
   false, /* bool dumpNetworkReceiverOperations */
   utilDebugSettings_init/* void (* init)( const utilCmdLine cmdLine ) */
};

utilDebugSettings_t * utilDebugSettings = &theDebugSettings;
