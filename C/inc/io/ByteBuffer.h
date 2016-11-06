#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <util/Status.h>
#include "sockets.h"

#include <stdio.h>

typedef enum ioByteOrder_e {

   ioByteOrder_BIG_ENDIAN,
   ioByteOrder_LITTLE_ENDIAN

} ioByteOrder;

UTIL_ADT( ioByteBuffer );

utilStatus ioByteBuffer_wrap       ( ioByteBuffer * This, size_t capacity, byte * array );
utilStatus ioByteBuffer_new        ( ioByteBuffer * This, size_t capacity );
utilStatus ioByteBuffer_delete     ( ioByteBuffer * This );
utilStatus ioByteBuffer_copy       ( ioByteBuffer   This, ioByteBuffer * to, size_t length );
utilStatus ioByteBuffer_array      ( ioByteBuffer   This, byte ** to );
utilStatus ioByteBuffer_clear      ( ioByteBuffer   This );
utilStatus ioByteBuffer_mark       ( ioByteBuffer   This );
utilStatus ioByteBuffer_reset      ( ioByteBuffer   This );
utilStatus ioByteBuffer_flip       ( ioByteBuffer   This );
utilStatus ioByteBuffer_getPosition( ioByteBuffer   This, size_t * position );
utilStatus ioByteBuffer_setPosition( ioByteBuffer   This, size_t   position );
utilStatus ioByteBuffer_getLimit   ( ioByteBuffer   This, size_t * limit );
utilStatus ioByteBuffer_remaining  ( ioByteBuffer   This, size_t * remaining );
utilStatus ioByteBuffer_put        ( ioByteBuffer   This, const byte * src, size_t from, size_t to );
utilStatus ioByteBuffer_get        ( ioByteBuffer   This, byte * target, size_t from, size_t to );
utilStatus ioByteBuffer_putByte    ( ioByteBuffer   This, byte value );
utilStatus ioByteBuffer_getByte    ( ioByteBuffer   This, byte * target );
utilStatus ioByteBuffer_putShort   ( ioByteBuffer   This, short value );
utilStatus ioByteBuffer_putUShort  ( ioByteBuffer   This, unsigned short value );
utilStatus ioByteBuffer_getShort   ( ioByteBuffer   This, short * target );
utilStatus ioByteBuffer_getUShort  ( ioByteBuffer   This, unsigned short * target );
utilStatus ioByteBuffer_putInt     ( ioByteBuffer   This, int value );
utilStatus ioByteBuffer_putUInt    ( ioByteBuffer   This, unsigned int value );
utilStatus ioByteBuffer_putUIntAt  ( ioByteBuffer   This, unsigned int value, size_t index );
utilStatus ioByteBuffer_getInt     ( ioByteBuffer   This, int * target );
utilStatus ioByteBuffer_getUInt    ( ioByteBuffer   This, unsigned int * target );
utilStatus ioByteBuffer_putLong    ( ioByteBuffer   This, int64_t value );
utilStatus ioByteBuffer_putULong   ( ioByteBuffer   This, uint64_t value );
utilStatus ioByteBuffer_getLong    ( ioByteBuffer   This, int64_t * target );
utilStatus ioByteBuffer_getULong   ( ioByteBuffer   This, uint64_t * target );
utilStatus ioByteBuffer_putFloat   ( ioByteBuffer   This, float value );
utilStatus ioByteBuffer_getFloat   ( ioByteBuffer   This, float * target );
utilStatus ioByteBuffer_putDouble  ( ioByteBuffer   This, double value );
utilStatus ioByteBuffer_getDouble  ( ioByteBuffer   This, double * target );
utilStatus ioByteBuffer_putString  ( ioByteBuffer   This, const char * src );
utilStatus ioByteBuffer_getString  ( ioByteBuffer   This, char * target, size_t sizeOfTarget );
utilStatus ioByteBuffer_putBuffer  ( ioByteBuffer   This, ioByteBuffer source );
utilStatus ioByteBuffer_send       ( ioByteBuffer   This, SOCKET sckt );
utilStatus ioByteBuffer_sendTo     ( ioByteBuffer   This, SOCKET sckt, struct sockaddr_in * target );
utilStatus ioByteBuffer_receive    ( ioByteBuffer   This, SOCKET sckt );
utilStatus ioByteBuffer_dump       ( ioByteBuffer   This, FILE * target );

#ifdef __cplusplus
}
#endif
