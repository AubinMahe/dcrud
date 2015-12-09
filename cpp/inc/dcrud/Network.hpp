#pragma once

#include <string>

namespace dcrud {

   class IParticipant;

   class Network {
   public:

      static IParticipant & join(
         const char *   networkConfFile,
         const char *   intrfcName,
         unsigned short id             );
   };
}
