#pragma once
#include <io/ByteBuffer.h>

UTIL_ADT( dcrudClassID );

dcrudClassID dcrudClassID_init       ( byte package1, byte package2, byte package3, byte clazz );
ioStatus     dcrudClassID_unserialize( ioByteBuffer source, dcrudClassID * target );
ioStatus     dcrudClassID_serialize  ( const dcrudClassID This, ioByteBuffer target );
bool         dcrudClassID_toString   ( const dcrudClassID This, char * target, size_t targetSize );
int          dcrudClassID_compareTo  ( const dcrudClassID * left, const dcrudClassID * right );
