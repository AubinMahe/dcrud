#pragma once
#include <util/types.h>
#include <io/socket.h>
#include <io/Status.h>

typedef enum ioByteOrder_e {

   ioByteOrder_BIG_ENDIAN,
   ioByteOrder_LITTLE_ENDIAN

} ioByteOrder;

UTIL_ADT( ioByteBuffer );

ioByteBuffer ioByteBuffer_new        ( size_t         capacity );
void         ioByteBuffer_delete     ( ioByteBuffer * This );
void         ioByteBuffer_clear      ( ioByteBuffer   This );
void         ioByteBuffer_mark       ( ioByteBuffer   This );
ioStatus     ioByteBuffer_reset      ( ioByteBuffer   This );
void         ioByteBuffer_flip       ( ioByteBuffer   This );
size_t       ioByteBuffer_getPosition( ioByteBuffer   This );
ioStatus     ioByteBuffer_setPosition( ioByteBuffer   This, size_t position );
size_t       ioByteBuffer_getLimit   ( ioByteBuffer   This );
size_t       ioByteBuffer_remaining  ( ioByteBuffer   This );
ioStatus     ioByteBuffer_put        ( ioByteBuffer   This, const byte * src, size_t from, size_t to );
ioStatus     ioByteBuffer_get        ( ioByteBuffer   This, byte * target, size_t from, size_t to );
ioStatus     ioByteBuffer_putByte    ( ioByteBuffer   This, byte value );
ioStatus     ioByteBuffer_getByte    ( ioByteBuffer   This, byte * target );
ioStatus     ioByteBuffer_putShort   ( ioByteBuffer   This, unsigned short value );
ioStatus     ioByteBuffer_getShort   ( ioByteBuffer   This, unsigned short * target );
ioStatus     ioByteBuffer_putInt     ( ioByteBuffer   This, unsigned int value );
ioStatus     ioByteBuffer_putIntAt   ( ioByteBuffer   This, unsigned int value, unsigned int index );
ioStatus     ioByteBuffer_getInt     ( ioByteBuffer   This, unsigned int * target );
ioStatus     ioByteBuffer_putLong    ( ioByteBuffer   This, uint64_t value );
ioStatus     ioByteBuffer_getLong    ( ioByteBuffer   This, uint64_t * target );
ioStatus     ioByteBuffer_putDouble  ( ioByteBuffer   This, double value );
ioStatus     ioByteBuffer_getDouble  ( ioByteBuffer   This, double * target );
ioStatus     ioByteBuffer_putString  ( ioByteBuffer   This, const char * src );
ioStatus     ioByteBuffer_getString  ( ioByteBuffer   This, char * target, size_t sizeOfTarget );
ioStatus     ioByteBuffer_putBuffer  ( ioByteBuffer   This, ioByteBuffer source );
ioByteBuffer ioByteBuffer_duplicate  ( ioByteBuffer   This );
ioStatus     ioByteBuffer_send       ( ioByteBuffer   This, SOCKET sckt, struct sockaddr_in * target );
ioStatus     ioByteBuffer_receive    ( ioByteBuffer   This, SOCKET sckt );
byte *       ioByteBuffer_getBytes   ( ioByteBuffer   This );
ioByteBuffer ioByteBuffer_copy       ( ioByteBuffer   This, size_t length );
