#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <dcrud/IParticipant.h>

dcrudIParticipant dcrudNetwork_join(
   unsigned int   publisherId,
   const char *   mcastAddr,
   unsigned short port,
   const char *   intrfc,
   bool           dumpReceivedBuffer );

void dcrudNetwork_leave( dcrudIParticipant * participant );

#ifdef __cplusplus
}
#endif
