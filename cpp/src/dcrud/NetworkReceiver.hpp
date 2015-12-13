#pragma once

#include "ParticipantImpl.hpp"

namespace dcrud {

   struct NetworkReceiver;
   struct ParticipantImpl;

   NetworkReceiver * createNetworkReceiver(
         ParticipantImpl & participant,
         const char *      address,
         unsigned short    port,
         const char *      intrfc );

   void deleteNetworkReceiver( NetworkReceiver * This );
}
