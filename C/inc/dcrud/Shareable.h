#pragma once
#include <io/ByteBuffer.h>
#include <dcrud/GUID.h>
#include <util/types.h>

UTIL_ADT( dcrudShareable );

typedef void * dcrudSerializable;
typedef void    (* dcrudShareable_setF )( dcrudSerializable This, const dcrudShareable source );
typedef ioError (* dcrudShareable_serializeF  )( dcrudSerializable This, ioByteBuffer target );
typedef ioError (* dcrudShareable_unserializeF)( dcrudSerializable This, ioByteBuffer source );

void dcrudShareable_init(
   dcrudSerializable           serializable,
   dcrudShareable *            baseInSerializable,
   dcrudGUID                   id,
   dcrudShareable_setF         set,
   dcrudShareable_serializeF   serialize,
   dcrudShareable_unserializeF unserialize );
dcrudGUID dcrudShareable_getId      ( dcrudShareable This );
void      dcrudShareable_set        ( dcrudShareable This, dcrudShareable source );
ioError   dcrudShareable_serialize  ( dcrudShareable This, ioByteBuffer target );
ioError   dcrudShareable_unserialize( dcrudShareable This, ioByteBuffer source );
