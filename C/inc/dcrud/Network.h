#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <dcrud/IParticipant.h>

utilStatus dcrudNetwork_join(
   unsigned int                id,
   const ioInetSocketAddress * addr,
   const char *                intrfc,
   dcrudIParticipant *         participant );
bool       dcrudNetwork_isAlive( void );
utilStatus dcrudNetwork_leave  ( void );

#ifdef __cplusplus
}
#endif
