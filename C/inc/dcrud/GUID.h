#pragma once
#include <io/ByteBuffer.h>

UTIL_ADT( dcrudGUID );


dcrudGUID dcrudGUID_init         ( const char * topic, int classId );
ioError   dcrudGUID_unserialize  ( ioByteBuffer source, dcrudGUID * target );
ioError   dcrudGUID_serialize    ( const dcrudGUID This, ioByteBuffer target );
int       dcrudGUID_getClassId   ( const dcrudGUID This );
int       dcrudGUID_getInstanceId( const dcrudGUID This );
bool      dcrudGUID_isShared     ( const dcrudGUID This );
bool      dcrudGUID_toString     ( const dcrudGUID This, char * target, size_t targetSize );
int       dcrudGUID_compareTo    ( const dcrudGUID * left, const dcrudGUID * right );
