#pragma once
#include <io/ByteBuffer.h>

DCRUD_ADT( dcrudGUID );

bool dcrudGUID_isValid    ( const dcrudGUID This );
bool dcrudGUID_matchSource( const dcrudGUID This, int source );
bool dcrudGUID_toString   ( const dcrudGUID This, char * target, size_t targetSize );
void dcrudGUID_serialize  ( const dcrudGUID This, ioByteBuffer * target );
int  dcrudGUID_compareTo  ( const dcrudGUID * left, const dcrudGUID * right );
