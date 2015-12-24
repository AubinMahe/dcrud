#include <dcrud/Network.h>
#include "ParticipantImpl.h"

dcrudIParticipant dcrudNetwork_join(
   unsigned int   publisherId,
   const char *   mcastAddr,
   unsigned short port,
   const char *   intrfc )
{
   ParticipantImpl * retVal = NULL;

   if( !intrfc ) {
      fprintf( stderr, "%s:%d:Network interface name can't be null\n", __FILE__, __LINE__ );
      return 0;
   }
   if( DCRUD_NO_ERROR != ParticipantImpl_new( publisherId, mcastAddr, port, intrfc, &retVal )) {
      dcrudIParticipant p = (dcrudIParticipant)retVal;
      dcrudIParticipant_delete( &p );
      retVal = 0;
   }
   return (dcrudIParticipant)retVal;
}
