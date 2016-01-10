#pragma once

#include "ParticipantImpl.h"
#include <io/InetSocketAddress.h>

UTIL_ADT( NetworkReceiver );

NetworkReceiver NetworkReceiver_new(
   ParticipantImpl *           participant,
   const ioInetSocketAddress * inetSocketAddress,
   const char *                intrfc,
   bool                        dumpReceivedBuffer );

void NetworkReceiver_delete( NetworkReceiver * This );
