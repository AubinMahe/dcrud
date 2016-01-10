#pragma once

#include <dcrud/IRegistry.hpp>
#include "Settings.h"

class StaticRegistry : public dcrud::IRegistry {
public:

   StaticRegistry( void ) {
      _participants.insert( io::InetSocketAddress( MCAST_ADDRESS, 2416 ));
      _participants.insert( io::InetSocketAddress( MCAST_ADDRESS, 2417 ));
   }

   virtual const dcrud::socketAddresses_t & getParticipants( void ) const {
      return _participants;
   }

   virtual const dcrud::socketAddresses_t & getClients( const std::string&/*interfaceName*/) const {
      return _clients;
   }

   virtual const dcrud::socketAddresses_t & getConsumers( const std::string&/*dataName*/) const {
      return _consumers;
   }

private:

   dcrud::socketAddresses_t _participants;
   dcrud::socketAddresses_t _clients;
   dcrud::socketAddresses_t _consumers;
};
