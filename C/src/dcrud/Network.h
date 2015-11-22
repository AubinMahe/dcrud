#pragma once
#include <dcrud/IParticipant.h>
#include <coll/Set.h>

dcrudIParticipant Network_Network(
   const char *   address,
   const char *   intrfc,
   unsigned short port,
   byte           platformId,
   byte           execId     );

void Network_delete(
   dcrudIParticipant * This );

unsigned int Network_getMCastAddress(
   dcrudIParticipant This );

void Network_publish(
   dcrudIParticipant This,
   byte              cacheId,
   collSet           updated,
   collSet           deleted );

dcrudShareable Network_newInstance(
   dcrudIParticipant This,
   dcrudClassID *    classId,
   ioByteBuffer      frame   );

unsigned int call(
   dcrudIParticipant This,
   const char *      intrfcName,
   const char *      opName,
   collMapVoidPtr    in,
   dcrudICallback    callback );
