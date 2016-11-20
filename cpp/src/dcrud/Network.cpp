#include <dcrud/Network.hpp>
#include <io/NetworkInterfaces.h>

#include "ParticipantImpl.hpp"

using namespace dcrud;

IParticipant & Network::join(
   unsigned int                  publisherId,
   const io::InetSocketAddress & addr,
   const std::string &           intrfc )
{
   return *new ParticipantImpl( publisherId, addr, intrfc );
}

void Network::leave( IParticipant & participant ) {
   delete &participant;
}
