#include "StaticRegistry.hpp"

StaticRegistry::StaticRegistry( void ) :
   _participants(), // dcrud::socketAddresses_t
   _clients     (), // dcrud::socketAddresses_t
   _consumers   ()  // dcrud::socketAddresses_t
{
   _participants.insert( io::InetSocketAddress( MCAST_ADDRESS, 2416 ));
   _participants.insert( io::InetSocketAddress( MCAST_ADDRESS, 2417 ));
}

StaticRegistry::~ StaticRegistry() {
   /**/
}
