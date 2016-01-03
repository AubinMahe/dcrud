#pragma once

#include <string>

namespace dcrud {

   class IParticipant;

   class Network {
   public:

      static IParticipant & join(
         unsigned int        publisherId,
         const std::string & mcastAddr,
         unsigned short      port,
         const std::string & intrfc );

      static void leave( IParticipant & participant );
   };
}
