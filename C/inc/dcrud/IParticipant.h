#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <io/InetSocketAddress.h>
#include "ICache.h"
#include "IDispatcher.h"
#include "IRegistry.h"

typedef dcrudShareable(* dcrudShareableFactory )();

UTIL_ADT( dcrudIParticipant );

typedef void     (* dcrudLocalFactory_Set         )( dcrudShareableData This, const dcrudShareableData source );
typedef ioStatus (* dcrudLocalFactory_Serialize   )( dcrudShareableData This, ioByteBuffer target );
typedef ioStatus (* dcrudLocalFactory_Unserialize )( dcrudShareableData This, ioByteBuffer source );
typedef bool     (* dcrudLocalFactory_Initialize  )( dcrudShareableData This );
struct dcrudRemoteFactory_s;
typedef void     (* dcrudRemoteFactory_create     )( struct dcrudRemoteFactory_s * This, dcrudArguments how );
typedef void     (* dcrudRemoteFactory_update     )( struct dcrudRemoteFactory_s * This, dcrudShareable what, dcrudArguments how );
typedef void     (* dcrudRemoteFactory_delete     )( struct dcrudRemoteFactory_s * This, dcrudShareable what );

typedef struct dcrudLocalFactory_s {

   dcrudClassID                  classID;
   size_t                        size;
   dcrudLocalFactory_Initialize  initialize;
   dcrudLocalFactory_Set         set;
   dcrudLocalFactory_Serialize   serialize;
   dcrudLocalFactory_Unserialize unserialize;

} dcrudLocalFactory;

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
void             dcrudIParticipant_listen               ( dcrudIParticipant   This, dcrudIRegistry registry, const char * intrfc, bool dumpReceivedBuffer );
bool             dcrudIParticipant_registerLocalFactory ( dcrudIParticipant   This, dcrudLocalFactory * local );
bool             dcrudIParticipant_registerRemoteFactory( dcrudIParticipant   This, dcrudRemoteFactory * remote );
dcrudICache      dcrudIParticipant_getDefaultCache      ( dcrudIParticipant   This );
dcrudStatus      dcrudIParticipant_createCache          ( dcrudIParticipant   This, dcrudICache * target, byte * cacheId );
dcrudICache      dcrudIParticipant_getCache             ( dcrudIParticipant   This, byte ID );
dcrudIDispatcher dcrudIParticipant_getDispatcher        ( dcrudIParticipant   This );
dcrudShareable   dcrudIParticipant_createShareable      ( dcrudIParticipant   This, dcrudClassID classID );
void             dcrudIParticipant_run                  ( dcrudIParticipant   This );

#ifdef __cplusplus
}
#endif
