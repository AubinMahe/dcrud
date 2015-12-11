#pragma once

namespace dcrud {

   struct IParticipant;

   struct Network {

      static IParticipant & join(
         const char *   networkConfFile,
         const char *   intrfcName,
         unsigned short id             );

      static void leave( IParticipant & toDelete );
   };
}
