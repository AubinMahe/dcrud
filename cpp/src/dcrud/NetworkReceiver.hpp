#pragma once

#include "ParticipantImpl.hpp"

namespace dcrud {

   class NetworkReceiver;
   class ParticipantImpl;

   NetworkReceiver * createNetworkReceiver(
         ParticipantImpl & participant,
         const char *      address,
         unsigned short    port,
         const char *      intrfc );

   void deleteNetworkReceiver( NetworkReceiver * This );
}
