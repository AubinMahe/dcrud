#pragma once
#include <dcrud/IParticipant.h>

dcrudIParticipant dcrudNetworks_join(
   const char *   address,
   const char *   intrfc,
   unsigned short port,
   byte           platformId,
   byte           execId     );
