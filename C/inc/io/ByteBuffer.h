#pragma once
#include <util/types.h>

typedef enum dcrudByteOrder_e {

   dcrudByteOrder_BIG_ENDIAN,
   dcrudByteOrder_LITTLE_ENDIAN

} dcrudByteOrder;

typedef struct dcrudByteBuffer_s {

   dcrudByteOrder order;
   unsigned int   position;
   unsigned int   limit;
   unsigned int   capacity;
   byte *         bytes;

} ioByteBuffer;

void           dcrudByteBuffer_init     ( ioByteBuffer * This, unsigned int capacity );
void           dcrudByteBuffer_setOrder ( ioByteBuffer * This, dcrudByteOrder order );
dcrudByteOrder dcrudByteBuffer_order    ( ioByteBuffer * This );
void           dcrudByteBuffer_clear    ( ioByteBuffer * This );
void           dcrudByteBuffer_flip     ( ioByteBuffer * This );
unsigned       dcrudByteBuffer_remaining( ioByteBuffer * This );
void           dcrudByteBuffer_put      ( ioByteBuffer * This, byte * bytes, unsigned int count );
void           dcrudByteBuffer_putInt   ( ioByteBuffer * This, unsigned value );
void           dcrudByteBuffer_putDouble( ioByteBuffer * This, double value );
void           dcrudByteBuffer_putBuffer( ioByteBuffer * This, ioByteBuffer * source );
