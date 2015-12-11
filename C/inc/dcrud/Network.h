#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <dcrud/IParticipant.h>

dcrudIParticipant dcrudNetwork_join(
   const char *   networkConfFile,
   const char *   intrfc,
   unsigned short id       );

void dcrudNetwork_leave( dcrudIParticipant * This );

#ifdef __cplusplus
}
#endif
