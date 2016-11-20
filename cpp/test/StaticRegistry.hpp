#pragma once

#include <dcrud/IRegistry.hpp>
#include "Settings.h"

class StaticRegistry : public dcrud::IRegistry {
public:

   StaticRegistry( void );
   ~ StaticRegistry();

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
