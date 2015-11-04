#include <io/ByteBuffer.h>
#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <winsock2.h>
#endif

static bool init = true;
static bool hostIsBigEndian = false;

void dcrudByteBuffer_init( ioByteBuffer * This, unsigned int capacity ) {
   if( init ) {
      init = false;
      hostIsBigEndian = ( htonl(1) == 1 );
   }
   This->order    = dcrudByteOrder_BIG_ENDIAN;
   This->position = 0;
   This->limit    = capacity;
   This->capacity = capacity;
   This->bytes    = (byte*)malloc( capacity );
}

void dcrudByteBuffer_setOrder( ioByteBuffer * This, dcrudByteOrder order ) {
   This->order = order;
}

dcrudByteOrder dcrudByteBuffer_order( ioByteBuffer * This ) {
   return This->order;
}

void dcrudByteBuffer_clear( ioByteBuffer * This ) {
   This->position = 0;
   This->limit    = This->capacity;
}

void dcrudByteBuffer_flip( ioByteBuffer * This ) {
   This->limit    = This->position;
   This->position = 0;
}

unsigned dcrudByteBuffer_remaining( ioByteBuffer * This ) {
   return This->limit - This->position;
}

void dcrudByteBuffer_put( ioByteBuffer * This, byte * bytes, unsigned int count ) {
   memcpy( This->bytes + This->position, bytes, count );
   This->position += count;
}

void dcrudByteBuffer_putInt( ioByteBuffer * This, unsigned value ) {
   unsigned ton;
   if( This->order == dcrudByteOrder_LITTLE_ENDIAN ) {
      if( hostIsBigEndian ) {
         ton = 0;
         ton |= (value & 0x000000FF) << 24;
         ton |= (value & 0x0000FF00) << 8;
         ton |= (value & 0x00FF0000) >> 8;
         ton |= (value & 0xFF000000) >> 24;
      }
      else {
         ton = value;
      }
   }
   else {
      ton = htonl( value );
   }
   memcpy( This->bytes + This->position, &ton, 4 );
   This->position += 4;
}

void dcrudByteBuffer_putDouble( ioByteBuffer * This, double value ) {
   if( hostIsBigEndian ) {
      memcpy( This->bytes + This->position, &value, 8 );
   }
   else {
      byte * ton = (byte *)&value;
      byte   tmp0 = ton[0];
      byte   tmp1 = ton[1];
      byte   tmp2 = ton[2];
      byte   tmp3 = ton[3];
      ton[0] = ton[7];
      ton[1] = ton[6];
      ton[2] = ton[5];
      ton[3] = ton[4];
      ton[4] = tmp3;
      ton[5] = tmp2;
      ton[6] = tmp1;
      ton[7] = tmp0;
      memcpy( This->bytes + This->position, ton, 8 );
   }
   This->position += 8;
}

void dcrudByteBuffer_putByteBuffer( ioByteBuffer * This, ioByteBuffer * source ) {
   unsigned count = source->limit - source->position;
   memcpy( This->bytes + This->position, source->bytes + source->position, count );
   source->position += count;
   This  ->position += count;
}
