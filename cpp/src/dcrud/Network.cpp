#include <dcrud/Network.hpp>
#include "ParticipantImpl.hpp"

using namespace dcrud;

IParticipant * Network::join(
   unsigned int        publisherId,
   const std::string & mcastAddr,
   unsigned short      port,
   const std::string & intrfc )
{
   return new ParticipantImpl( publisherId, mcastAddr, port, intrfc );
}
