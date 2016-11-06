#include <dcrud/Network.h>

#include "ParticipantImpl.h"

static bool isAlive = false;

utilStatus dcrudNetwork_join(
   unsigned int                publisherId,
   const ioInetSocketAddress * addr,
   const char *                intrfc,
   dcrudIParticipant *         target )
{
   utilStatus              status      = UTIL_STATUS_NO_ERROR;
   dcrudIParticipantImpl * participant = NULL;
   if( ! intrfc ) {
      return UTIL_STATUS_NULL_ARGUMENT;
   }
   status = dcrudIParticipantImpl_new( &participant, publisherId, addr, intrfc );
   if( UTIL_STATUS_NO_ERROR == status ) {
      *target = (dcrudIParticipant)participant;
      isAlive = true;
   }
   return status;
}

bool dcrudNetwork_isAlive( void ) {
   return isAlive;
}

utilStatus dcrudNetwork_leave( void ) {
   isAlive = false;
   return UTIL_STATUS_NO_ERROR;
}
