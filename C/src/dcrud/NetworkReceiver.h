#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "ParticipantImpl.h"
#include <io/InetSocketAddress.h>

UTIL_ADT( dcrudNetworkReceiver );

utilStatus dcrudNetworkReceiver_new(
   dcrudNetworkReceiver *      This,
   dcrudIParticipantImpl *     participant,
   const ioInetSocketAddress * inetSocketAddress,
   const char *                intrfc,
   bool                        dumpReceivedBuffer );

utilStatus dcrudNetworkReceiver_delete(
   dcrudNetworkReceiver *      This );

#ifdef __cplusplus
}
#endif
