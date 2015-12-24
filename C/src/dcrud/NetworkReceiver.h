#pragma once

#include "ParticipantImpl.h"

UTIL_ADT( NetworkReceiver );

NetworkReceiver NetworkReceiver_new(
   ParticipantImpl * participant,
   const char *      address,
   unsigned short    port,
   const char *      intrfc );

void NetworkReceiver_delete( NetworkReceiver * This );
