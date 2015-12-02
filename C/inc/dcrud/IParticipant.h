#pragma once
#include <dcrud/ICache.h>
#include <dcrud/IDispatcher.h>

typedef dcrudShareable(* dcrudShareableFactory )();

UTIL_ADT( dcrudIParticipant );

typedef void     (* dcrudShareable_Set         )( dcrudShareableData This, const dcrudShareableData source );
typedef ioStatus (* dcrudShareable_Serialize   )( dcrudShareableData This, ioByteBuffer target );
typedef ioStatus (* dcrudShareable_Unserialize )( dcrudShareableData This, ioByteBuffer source );
typedef bool     (* dcrudShareable_Initialize  )( dcrudShareableData This );

bool             dcrudIParticipant_registerClass  ( dcrudIParticipant This,
   dcrudClassID               id,
   size_t                     size,
   dcrudShareable_Initialize  initialize,
   dcrudShareable_Set         set,
   dcrudShareable_Serialize   serialize,
   dcrudShareable_Unserialize unserialize );
dcrudStatus      dcrudIParticipant_createCache    ( dcrudIParticipant   This, dcrudICache * target );
dcrudICache      dcrudIParticipant_getCache       ( dcrudIParticipant   This, byte ID );
dcrudIDispatcher dcrudIParticipant_getDispatcher  ( dcrudIParticipant   This );
dcrudShareable   dcrudIParticipant_createShareable( dcrudIParticipant   This, dcrudClassID classID );
void             dcrudIParticipant_run            ( dcrudIParticipant   This );
