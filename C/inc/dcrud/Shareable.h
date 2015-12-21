#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <io/ByteBuffer.h>

#include "GUID.h"
#include "ClassID.h"

UTIL_ADT( dcrudShareable );

typedef void * dcrudShareableData;

void               dcrudShareable_delete      ( dcrudShareable * This );
dcrudGUID          dcrudShareable_getGUID     ( dcrudShareable   This );
dcrudClassID       dcrudShareable_getClassID  ( dcrudShareable   This );
void               dcrudShareable_set         ( dcrudShareable   This, dcrudShareable source );
int                dcrudShareable_compareTo   ( dcrudShareable * left, dcrudShareable * right );
dcrudShareableData dcrudShareable_getUserData ( dcrudShareable   This );
dcrudShareable     dcrudShareable_getShareable( dcrudShareableData user );

#ifdef __cplusplus
}
#endif
