#include <dcrud/Network.h>
#include <io/NetworkInterfaces.h>
#include <util/Trace.h>

#include "ParticipantImpl.h"

dcrudIParticipant dcrudNetwork_join(
   unsigned int                publisherId,
   const ioInetSocketAddress * addr,
   const char *                intrfc )
{
   ParticipantImpl * retVal = NULL;
   dcrudStatus status = DCRUD_NO_ERROR;

   if( ! intrfc ) {
      intrfc = ioNetworkInterfaces_getFirst( true );
   }
   utilTrace_entry( __func__, "publisherId: %d, intrfc: '%s'\n", publisherId, intrfc );
   status = ParticipantImpl_new( publisherId, addr, intrfc, &retVal );
   if( DCRUD_NO_ERROR != status ) {
      dcrudIParticipant p = (dcrudIParticipant)retVal;
      utilTrace_error( __func__, "ParticipantImpl_new returns %d", status );
      dcrudIParticipant_delete( &p );
      retVal = NULL;
   }
   utilTrace_exit( __func__, "retVal: %08X", retVal );
   return (dcrudIParticipant)retVal;
}

void dcrudNetwork_leave( dcrudIParticipant * participant ) {
   dcrudIParticipant_delete( participant );
}
