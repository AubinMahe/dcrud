#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <io/ByteBuffer.h>

#include "GUID.h"
#include "ClassID.h"

UTIL_ADT( dcrudShareable );

typedef void * dcrudShareableData;

typedef utilStatus(* dcrudLocalFactory_Allocate   )( dcrudShareableData * This );
typedef utilStatus(* dcrudLocalFactory_Serialize  )( dcrudShareableData   This, ioByteBuffer target );
typedef utilStatus(* dcrudLocalFactory_Unserialize)( dcrudShareableData   This, ioByteBuffer source );
typedef utilStatus(* dcrudLocalFactory_Release    )( dcrudShareableData * This );

typedef struct dcrudLocalFactory_s {

   dcrudClassID                  classID;
   dcrudLocalFactory_Allocate    allocateUserData;
   dcrudLocalFactory_Serialize   serialize;
   dcrudLocalFactory_Unserialize unserialize;
   dcrudLocalFactory_Release     releaseUserData;

} dcrudLocalFactory;

utilStatus dcrudShareable_new       ( dcrudShareable * This, dcrudClassID classID, dcrudLocalFactory * factory, dcrudShareableData data );
utilStatus dcrudShareable_delete    ( dcrudShareable * This );
utilStatus dcrudShareable_getGUID   ( dcrudShareable   This, dcrudGUID * guid );
utilStatus dcrudShareable_getClassID( dcrudShareable   This, dcrudClassID * classID );
utilStatus dcrudShareable_set       ( dcrudShareable   This, dcrudShareable source );
utilStatus dcrudShareable_getData   ( dcrudShareable   This, dcrudShareableData * data );
utilStatus dcrudShareable_detach    ( dcrudShareable   This );
int        dcrudShareable_compareTo ( dcrudShareable * left, dcrudShareable * right );

#ifdef __cplusplus
}
#endif
