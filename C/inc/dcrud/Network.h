#pragma once
#include <dcrud/IParticipant.h>

dcrudIParticipant dcrudNetwork_join(
   const char *   networkConfFile,
   const char *   intrfc,
   unsigned short id       );

void dcrudNetwork_leave( dcrudIParticipant * This );
