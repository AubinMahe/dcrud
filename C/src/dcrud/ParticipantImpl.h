#pragma once
#include <dcrud/IParticipant.h>
#include <coll/Set.h>
#include <io/socket.h>
#include <os/Mutex.h>

#define DCRUD_SIGNATURE_SIZE  5U
extern const byte DCRUD_SIGNATURE[DCRUD_SIGNATURE_SIZE];
#define FRAME_TYPE_SIZE 1U
#define SIZE_SIZE       4U
#define GUID_SIZE       ( 1U + 1U + 1U + 4U )
#define CLASS_ID_SIZE   ( 1U + 1U + 1U + 1U )
#define HEADER_SIZE     ( SIZE_SIZE + FRAME_TYPE_SIZE + SIZE_SIZE + GUID_SIZE + CLASS_ID_SIZE )
#define PAYLOAD_SIZE    ( 64U*1024U - HEADER_SIZE )

typedef enum FrameType_e {

   FRAMETYPE_NO_OP,
   FRAMETYPE_DATA_CREATE_OR_UPDATE,
   FRAMETYPE_DATA_DELETE,
   FRAMETYPE_OPERATION

} FrameType;

typedef struct ParticipantImpl_s {

   osMutex            cachesMutex;
   dcrudICache        caches[256];
   ioByteBuffer       header;
   ioByteBuffer       payload;
   ioByteBuffer       message;
   osMutex            classesMutex;
   collMap            classes;
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
void           ParticipantImpl_delete         ( ParticipantImpl * * This );
unsigned int   ParticipantImpl_getMCastAddress( ParticipantImpl *   This );
void           ParticipantImpl_publishUpdated ( ParticipantImpl *   This, collSet updated );
void           ParticipantImpl_publishDeleted ( ParticipantImpl *   This, collSet deleted );
dcrudShareable ParticipantImpl_newInstance    ( ParticipantImpl *   This, ioByteBuffer frame );
void           ParticipantImpl_sendCall       ( ParticipantImpl *   This, const char * intrfcName, const char * opName, dcrudArguments args, int callId );
void           ParticipantImpl_call           ( ParticipantImpl *   This, const char * intrfcName, const char * opName, dcrudArguments args, dcrudICallback callback );
bool           ParticipantImpl_callback       ( ParticipantImpl *   This, const char * intrfcName, const char * opName, dcrudArguments args, int callId );
