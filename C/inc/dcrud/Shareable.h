#pragma once
#include <io/ByteBuffer.h>
#include <dcrud/GUID.h>
#include <util/types.h>

DCRUD_ADT( dcrudShareable );

typedef void * dcrudSerializable;
typedef void (* dcrudShareable_setF        )( dcrudSerializable This, const dcrudShareable source );
typedef void (* dcrudShareable_serializeF  )( dcrudSerializable This, ioByteBuffer * target );
typedef void (* dcrudShareable_unserializeF)( dcrudSerializable This, ioByteBuffer * source );

void dcrudShareable_init(
   dcrudSerializable           serializable,
   dcrudShareable *            baseInSerializable,
   int                         classId,
   dcrudShareable_setF         set,
   dcrudShareable_serializeF   serialize,
   dcrudShareable_unserializeF unserialize );
int       dcrudShareable_getClassId ( dcrudShareable This );
dcrudGUID dcrudShareable_getId      ( dcrudShareable This );
void      dcrudShareable_serialize  ( dcrudShareable This, ioByteBuffer * target );
void      dcrudShareable_unserialize( dcrudShareable This, ioByteBuffer * source );
