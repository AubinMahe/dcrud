#pragma once
#include <dcrud/IParticipant.h>

dcrudIParticipant dcrudNetwork_join(
   const char * networkConfFile,
   const char * intrfc,
   const char * name    );

void dcrudNetwork_leave( dcrudIParticipant * This );
