#pragma once
#include <io/ByteBuffer.h>
#include <dcrud/GUID.h>
#include <dcrud/ClassID.h>
#include <util/types.h>

UTIL_ADT( dcrudShareable );

typedef void * dcrudSerializable;
typedef void    (* dcrudShareable_setF )( dcrudSerializable This, const dcrudShareable source );
typedef ioStatus (* dcrudShareable_serializeF )( dcrudSerializable This, ioByteBuffer target );
typedef ioStatus (* dcrudShareable_unserializeF)( dcrudSerializable This, ioByteBuffer source );

void         dcrudShareable_init(
   dcrudSerializable           serializable,
   dcrudShareable *            baseInSerializable,
   dcrudShareable_setF         set,
   dcrudShareable_serializeF   serialize,
   dcrudShareable_unserializeF unserialize );
dcrudGUID    dcrudShareable_getGUID    ( dcrudShareable This );
dcrudClassID dcrudShareable_getClassID ( dcrudShareable This );
void         dcrudShareable_set        ( dcrudShareable This, dcrudShareable source );
ioStatus     dcrudShareable_serialize  ( dcrudShareable This, ioByteBuffer target );
ioStatus     dcrudShareable_unserialize( dcrudShareable This, ioByteBuffer source );
int          dcrudShareable_compareTo  ( dcrudShareable * left, dcrudShareable * right );
