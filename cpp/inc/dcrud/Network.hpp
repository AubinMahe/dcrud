#pragma once

#include <io/InetSocketAddress.hpp>

namespace dcrud {

   class IParticipant;

   class Network {
   public:

      /**
       *
       * @param id     participant id
       * @param addr   IP address:port pair
       * @param intrfc the network interface to use, identified by its IP address
       */
      static IParticipant & join(
         unsigned int                  id,
         const io::InetSocketAddress & addr,
         const std::string &           intrfc );

      /**
       *
       * @param id   participant id
       * @param addr IP address:port pair
       * *Note*: The first up, non loopback, multicast capable interface will be used
       */
      static IParticipant & join( unsigned int id, const io::InetSocketAddress & addr );

      static void leave( IParticipant & participant );
   };
}
