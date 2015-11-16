#pragma once
#include <util/types.h>
#include <io/socket.h>
#include <io/Errors.h>

typedef enum ioByteOrder_e {

   ioByteOrder_BIG_ENDIAN,
   ioByteOrder_LITTLE_ENDIAN

} ioByteOrder;

UTIL_ADT( ioByteBuffer );

ioByteBuffer ioByteBuffer_allocate ( size_t capacity );
void    ioByteBuffer_clear    ( ioByteBuffer This );
void    ioByteBuffer_flip     ( ioByteBuffer This );
size_t  ioByteBuffer_remaining( ioByteBuffer This );
ioError ioByteBuffer_put      ( ioByteBuffer This, const byte * src, size_t from, size_t to );
ioError ioByteBuffer_get      ( ioByteBuffer This, byte * target, size_t from, size_t to );
ioError ioByteBuffer_putShort ( ioByteBuffer This, unsigned short value );
ioError ioByteBuffer_getShort ( ioByteBuffer This, unsigned short * target );
ioError ioByteBuffer_putInt   ( ioByteBuffer This, unsigned int value );
ioError ioByteBuffer_putIntAt ( ioByteBuffer This, unsigned int value, unsigned int index );
ioError ioByteBuffer_getInt   ( ioByteBuffer This, unsigned int * target );
ioError ioByteBuffer_putLong  ( ioByteBuffer This, uint64_t value );
ioError ioByteBuffer_getLong  ( ioByteBuffer This, uint64_t * target );
ioError ioByteBuffer_putDouble( ioByteBuffer This, double value );
ioError ioByteBuffer_getDouble( ioByteBuffer This, double * target );
ioError ioByteBuffer_putString( ioByteBuffer This, const char * src );
ioError ioByteBuffer_getString( ioByteBuffer This, char * target, size_t sizeOfTarget );
ioError ioByteBuffer_putBuffer( ioByteBuffer This, ioByteBuffer source );
ioError ioByteBuffer_sendTo   (
   ioByteBuffer         This,
   SOCKET               sckt,
   struct sockaddr_in * target,
   int *                countBytes );
