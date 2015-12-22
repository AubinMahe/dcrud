#pragma once
#include <dcrud/IParticipant.h>
#include <coll/Set.h>
#include <io/socket.h>
#include <os/Mutex.h>

#define CACHES_COUNT    256

typedef struct ParticipantImpl_s {

   osMutex            cachesMutex;
   dcrudICache        caches[CACHES_COUNT];
   unsigned int       nextCacheID;
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
   unsigned short     publisherId;
   dcrudIDispatcher   dispatcher;
   unsigned int       itemCount;
   int                callId;

} ParticipantImpl;

dcrudStatus    ParticipantImpl_new            ( unsigned short publisherId, const char * address, unsigned short port, const char * intrfc, ParticipantImpl * * target );
unsigned int   ParticipantImpl_getMCastAddress( ParticipantImpl *   This );
void           ParticipantImpl_publishUpdated ( ParticipantImpl *   This, collSet updated );
void           ParticipantImpl_publishDeleted ( ParticipantImpl *   This, collSet deleted );
dcrudShareable ParticipantImpl_newInstance    ( ParticipantImpl *   This, ioByteBuffer frame );
void           ParticipantImpl_sendCall       ( ParticipantImpl *   This, const char * intrfcName, const char * opName, dcrudArguments args, int callId );
void           ParticipantImpl_call           ( ParticipantImpl *   This, const char * intrfcName, const char * opName, dcrudArguments args, dcrudICallback callback );
bool           ParticipantImpl_callback       ( ParticipantImpl *   This, const char * intrfcName, const char * opName, dcrudArguments args, int callId );
bool           ParticipantImpl_create         ( ParticipantImpl *   This, dcrudClassID clsId, dcrudArguments how );
bool           ParticipantImpl_update         ( ParticipantImpl *   This, dcrudGUID id, dcrudArguments how );
bool           ParticipantImpl_delete         ( ParticipantImpl *   This, dcrudGUID id );
