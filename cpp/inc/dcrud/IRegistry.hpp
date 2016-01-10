#pragma once

#include <io/InetSocketAddress.hpp>

namespace dcrud {

   typedef std::set<io::InetSocketAddress>   socketAddresses_t;
   typedef socketAddresses_t::const_iterator socketAddressesCstIter_t;

   class IRegistry {
   public:

      virtual ~ IRegistry(){}

      virtual const socketAddresses_t & getParticipants( void ) const = 0;
      virtual const socketAddresses_t & getClients     ( const std::string & interfaceName ) const = 0;
      virtual const socketAddresses_t & getConsumers   ( const std::string & dataName ) const = 0;
   };
}
