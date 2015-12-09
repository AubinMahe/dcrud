#pragma once

namespace dcrud {

   struct ParticipantImpl;

   struct NetworkReceiver {

      NetworkReceiver(
         ParticipantImpl & participant,
         const char *      address,
         unsigned short    port,
         const char *      intrfc );
   };
}
