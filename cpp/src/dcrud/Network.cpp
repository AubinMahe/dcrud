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

IParticipant & Network::join( unsigned int id, const io::InetSocketAddress & addr ) {
   std::string intrfc = ioNetworkInterfaces_getFirst( true );
   return *new ParticipantImpl( id, addr, intrfc );
}

void Network::leave( IParticipant & participant ) {
   delete &participant;
}
