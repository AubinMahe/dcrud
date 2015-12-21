#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <dcrud/IParticipant.h>

dcrudIParticipant dcrudNetwork_join(
   byte           id,
   const char *   mcastAddr,
   unsigned short port,
   const char *   intrfc );

#ifdef __cplusplus
}
#endif
