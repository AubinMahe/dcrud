#pragma once

#include "ParticipantImpl.h"

struct NetworkReceiver_s;

struct NetworkReceiver_s * createNetworkReceiver(
   ParticipantImpl * participant,
   const char *      address,
   unsigned short    port,
   const char *      intrfc );

void deleteNetworkReceiver( struct NetworkReceiver_s * This );
