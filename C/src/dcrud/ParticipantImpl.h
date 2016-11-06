#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <dcrud/IParticipant.h>

#include <coll/Map.h>
#include <coll/List.h>
#include <io/sockets.h>
#include <os/Mutex.h>

#define CACHES_COUNT    256

typedef struct ParticipantImpl_s {

   unsigned int       magic;
   osMutex            cachesMutex;
   dcrudICache        caches[CACHES_COUNT];
   ioByteBuffer       header;
   ioByteBuffer       payload;
   ioByteBuffer       message;
   osMutex            factoriesMutex;
   collMap            factories;
   osMutex            publishersMutex;
   collMap            publishers;
   collMap            callbacks;
   struct sockaddr_in target;
   osMutex            outMutex;
   SOCKET             out;
   unsigned int       publisherId;
   dcrudIDispatcher   dispatcher;
   unsigned int       cacheCount;
   int                callId;
   collList           receivers;

} dcrudIParticipantImpl;

utilStatus dcrudIParticipantImpl_new            ( dcrudIParticipantImpl ** This, unsigned int publisherId, const ioInetSocketAddress * addr, const char * intrfc );
utilStatus dcrudIParticipantImpl_getMCastAddress( dcrudIParticipantImpl *  This, unsigned int * address );
utilStatus dcrudIParticipantImpl_publishUpdated ( dcrudIParticipantImpl *  This, collSet updated );
utilStatus dcrudIParticipantImpl_publishDeleted ( dcrudIParticipantImpl *  This, collSet deleted );
utilStatus dcrudIParticipantImpl_newInstance    ( dcrudIParticipantImpl *  This, ioByteBuffer frame, dcrudShareableData data, dcrudShareable * shared );
utilStatus dcrudIParticipantImpl_sendCall       ( dcrudIParticipantImpl *  This, const char * intrfcName, const char * opName, dcrudArguments args, int callId );
utilStatus dcrudIParticipantImpl_call           ( dcrudIParticipantImpl *  This, const char * intrfcName, const char * opName, dcrudArguments args, dcrudICallback callback );
utilStatus dcrudIParticipantImpl_callback       ( dcrudIParticipantImpl *  This, const char * intrfcName, const char * opName, dcrudArguments args, int callId );
utilStatus dcrudIParticipantImpl_createData     ( dcrudIParticipantImpl *  This, dcrudClassID clsId, dcrudArguments how );
utilStatus dcrudIParticipantImpl_updateData     ( dcrudIParticipantImpl *  This, dcrudGUID id, dcrudArguments how );
utilStatus dcrudIParticipantImpl_deleteData     ( dcrudIParticipantImpl *  This, dcrudGUID id );
utilStatus dcrudIParticipantImpl_delete         ( dcrudIParticipantImpl ** This );

#ifdef __cplusplus
}
#endif
