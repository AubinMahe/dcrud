#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <io/InetSocketAddress.h>
#include "ICache.h"
#include "IDispatcher.h"
#include "IRegistry.h"

typedef dcrudShareable(* dcrudShareableFactory )( void );

UTIL_ADT( dcrudIParticipant );

struct dcrudRemoteFactory_s;
typedef utilStatus(* dcrudRemoteFactory_create)(struct dcrudRemoteFactory_s * This, dcrudArguments how );
typedef utilStatus(* dcrudRemoteFactory_update)(struct dcrudRemoteFactory_s * This, dcrudShareable what, dcrudArguments how );
typedef utilStatus(* dcrudRemoteFactory_delete)(struct dcrudRemoteFactory_s * This, dcrudShareable what );

typedef struct dcrudRemoteFactory_s {

   dcrudClassID              classID;
   void *                    userContext;
   dcrudIParticipant         participant;
   dcrudRemoteFactory_create create;
   dcrudRemoteFactory_update update;
   dcrudRemoteFactory_delete delete;

} dcrudRemoteFactory;

/**
 * Use the registry to instanciate and listen to other participants.
 * @param This     this participant
 * @param registry the registry to use
 * @param intrfc   the network interface to use, identified by its IP address, may be null,
 * in such case the first up, non loopback, multicast capable interface will be used
 * @param dumpReceivedBuffer if true, dump the received frames on stderr
 */
utilStatus dcrudIParticipant_listen               ( dcrudIParticipant   This, dcrudIRegistry registry, const char * intrfc );
utilStatus dcrudIParticipant_registerLocalFactory ( dcrudIParticipant   This, dcrudLocalFactory * local );
utilStatus dcrudIParticipant_registerRemoteFactory( dcrudIParticipant   This, dcrudRemoteFactory * remote );
utilStatus dcrudIParticipant_getDefaultCache      ( dcrudIParticipant   This, dcrudICache * cache );
utilStatus dcrudIParticipant_createCache          ( dcrudIParticipant   This, unsigned int * cacheId, dcrudICache * cache );
utilStatus dcrudIParticipant_getCache             ( dcrudIParticipant   This, unsigned int   cacheId, dcrudICache * cache );
utilStatus dcrudIParticipant_getDispatcher        ( dcrudIParticipant   This, dcrudIDispatcher * dispatcher );
utilStatus dcrudIParticipant_createShareable      ( dcrudIParticipant   This, dcrudClassID classID, dcrudShareableData data, dcrudShareable * shareable );
utilStatus dcrudIParticipant_run                  ( dcrudIParticipant   This );

#ifdef __cplusplus
}
#endif
