#pragma once

#include "ParticipantImpl.h"

UTIL_ADT( INetworkReceiver );

INetworkReceiver INetworkReceiver_new(
   ParticipantImpl * participant,
   const char *      address,
   unsigned short    port,
   const char *      intrfc );

void INetworkReceiver_delete( INetworkReceiver * This );
