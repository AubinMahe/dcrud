#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "ICache.h"
#include "IDispatcher.h"

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

void             dcrudIParticipant_listen               ( dcrudIParticipant   This, const char * mcastAddr, unsigned short port, const char * networkInterface );
bool             dcrudIParticipant_registerLocalFactory ( dcrudIParticipant   This, dcrudLocalFactory * local );
bool             dcrudIParticipant_registerRemoteFactory( dcrudIParticipant   This, dcrudRemoteFactory * remote );
dcrudICache      dcrudIParticipant_getDefaultCache      ( dcrudIParticipant   This );
dcrudStatus      dcrudIParticipant_createCache          ( dcrudIParticipant   This, dcrudICache * target, byte * cacheId );
dcrudICache      dcrudIParticipant_getCache             ( dcrudIParticipant   This, byte ID );
dcrudIDispatcher dcrudIParticipant_getDispatcher        ( dcrudIParticipant   This );
dcrudShareable   dcrudIParticipant_createShareable      ( dcrudIParticipant   This, dcrudClassID classID );
void             dcrudIParticipant_run                  ( dcrudIParticipant   This );
void             dcrudIParticipant_delete               ( dcrudIParticipant * This );

#ifdef __cplusplus
}
#endif
