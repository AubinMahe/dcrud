#pragma once
#include <io/ByteBuffer.h>
#include <dcrud/GUID.h>
#include <dcrud/ClassID.h>
#include <util/types.h>

UTIL_ADT( dcrudShareable );

typedef void * dcrudSerializable;

void              dcrudShareable_delete      ( dcrudShareable * This );
dcrudGUID         dcrudShareable_getGUID     ( dcrudShareable   This );
dcrudClassID      dcrudShareable_getClassID  ( dcrudShareable   This );
void              dcrudShareable_set         ( dcrudShareable   This, dcrudShareable source );
int               dcrudShareable_compareTo   ( dcrudShareable * left, dcrudShareable * right );
dcrudSerializable dcrudShareable_getUserData ( dcrudShareable   This );
dcrudShareable    dcrudShareable_getShareable( dcrudSerializable user );
