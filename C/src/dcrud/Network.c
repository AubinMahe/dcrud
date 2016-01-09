#include <dcrud/Network.h>
#include <util/Trace.h>

#include "ParticipantImpl.h"

dcrudIParticipant dcrudNetwork_join(
   unsigned int   publisherId,
   const char *   mcastAddr,
   unsigned short port,
   const char *   intrfc,
   bool           dumpReceivedBuffer )
{
   ParticipantImpl * retVal = NULL;
   dcrudStatus status = DCRUD_NO_ERROR;

   utilTrace_entry( __func__,
      "publisherId: %d, mcastAddr: '%s', port: %d, intrfc: '%s', dumpReceivedBuffer: %d",
      publisherId, mcastAddr, port, intrfc, dumpReceivedBuffer );
   if( !intrfc ) {
      fprintf( stderr, "%s:%d:Network interface name can't be null\n", __FILE__, __LINE__ );
      utilTrace_error( __func__, "Network interface name can't be null" );
      retVal = NULL;
   }
   else {
      status =
         ParticipantImpl_new( publisherId, mcastAddr, port, intrfc, dumpReceivedBuffer, &retVal );
      if( DCRUD_NO_ERROR != status ) {
         dcrudIParticipant p = (dcrudIParticipant)retVal;
         utilTrace_error( __func__, "ParticipantImpl_new returns %d", status );
         dcrudIParticipant_delete( &p );
         retVal = NULL;
      }
   }
   utilTrace_exit( __func__, "retVal: %08X", retVal );
   return (dcrudIParticipant)retVal;
}

void dcrudNetwork_leave( dcrudIParticipant * participant ) {
   dcrudIParticipant_delete( participant );
}
