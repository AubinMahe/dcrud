#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "ICache.h"
#include "IDispatcher.h"

typedef dcrudShareable(* dcrudShareableFactory )();

UTIL_ADT( dcrudIParticipant );

typedef struct dcrudCounterpart_s {

   const char *   mcastAddr;
   unsigned short port;

} dcrudCounterpart;

void dcrudIParticipant_listen(
   dcrudIParticipant  This,
   const char *       networkInterface,
   dcrudCounterpart * others[]         );

typedef void (* dcrudShareable_Set )( dcrudShareableData This, const dcrudShareableData source );
typedef ioStatus (* dcrudShareable_Serialize )( dcrudShareableData This, ioByteBuffer target );
typedef ioStatus (* dcrudShareable_Unserialize )( dcrudShareableData This, ioByteBuffer source );
typedef bool (* dcrudShareable_Initialize )( dcrudShareableData This );

typedef struct dcrudIFactory_s {

   dcrudClassID               classID;
   size_t                     size;
   dcrudShareable_Initialize  initialize;
   dcrudShareable_Set         set;
   dcrudShareable_Serialize   serialize;
   dcrudShareable_Unserialize unserialize;

} dcrudIFactory;

bool dcrudIParticipant_registerFactory( dcrudIParticipant This, dcrudIFactory * factory );

typedef void (* dcrudICRUDSrvc_create )( dcrudArguments how );
typedef void (* dcrudICRUDSrvc_update )( dcrudShareable what, dcrudArguments how );
typedef void (* dcrudICRUDSrvc_delete )( dcrudShareable what );

typedef struct dcrudICRUDSrvc_s {

   dcrudICRUDSrvc_create create;
   dcrudICRUDSrvc_update update;
   dcrudICRUDSrvc_delete delete;

} dcrudICRUDSrvc;

bool dcrudIParticipant_registerPublisher(
   dcrudIParticipant This,
   dcrudClassID      id,
   dcrudICRUDSrvc *  publisher );

dcrudICache dcrudIParticipant_getDefaultCache( dcrudIParticipant This );

dcrudStatus dcrudIParticipant_createCache(
   dcrudIParticipant This,
   dcrudICache *     target );

dcrudICache dcrudIParticipant_getCache( dcrudIParticipant This, byte ID );

dcrudIDispatcher dcrudIParticipant_getDispatcher( dcrudIParticipant This );

dcrudShareable dcrudIParticipant_createShareable( dcrudIParticipant This, dcrudClassID classID );

void dcrudIParticipant_run( dcrudIParticipant This );

void dcrudIParticipant_delete( dcrudIParticipant * This );

#ifdef __cplusplus
}
#endif
