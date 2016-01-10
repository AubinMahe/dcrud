#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <dcrud/IParticipant.h>

/**
 *
 * @param id     participant id
 * @param addr   IP address:port pair
 * @param intrfc the network interface to use, identified by its IP address, may be null
 * in such case the first up, non loopback, multicast capable interface will be used)
 */
dcrudIParticipant dcrudNetwork_join(
   unsigned int                id,
   const ioInetSocketAddress * addr,
   const char *                intrfc );

void dcrudNetwork_leave( dcrudIParticipant * participant );

#ifdef __cplusplus
}
#endif
