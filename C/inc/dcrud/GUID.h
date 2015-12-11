#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <io/ByteBuffer.h>

UTIL_ADT( dcrudGUID );

ioStatus dcrudGUID_unserialize( ioByteBuffer source, dcrudGUID * target );
ioStatus dcrudGUID_serialize  ( const dcrudGUID This, ioByteBuffer target );
bool     dcrudGUID_toString   ( const dcrudGUID This, char * target, size_t targetSize );
int      dcrudGUID_compareTo  ( const dcrudGUID * left, const dcrudGUID * right );

#ifdef __cplusplus
}
#endif
