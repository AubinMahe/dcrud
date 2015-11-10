#include <io/ByteBuffer.h>
#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <winsock2.h>
#elif __linux__
#  include <arpa/inet.h>
#  include <string.h>
#endif

#include <stdio.h>

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
   if( ( This->order == dcrudByteOrder_LITTLE_ENDIAN &&  hostIsBigEndian )
     ||( This->order == dcrudByteOrder_BIG_ENDIAN    && !hostIsBigEndian ))
   {
      unsigned ton = 0;
      ton |= (value & 0x000000FF) << 24;
      ton |= (value & 0x0000FF00) << 8;
      ton |= (value & 0x00FF0000) >> 8;
      ton |= (value & 0xFF000000) >> 24;
      memcpy( This->bytes + This->position, &ton, 4 );
   }
   else {
      memcpy( This->bytes + This->position, &value, 4 );
   }
   This->position += 4;
}

void dcrudByteBuffer_putDouble( ioByteBuffer * This, double value ) {
   if( ( This->order == dcrudByteOrder_LITTLE_ENDIAN &&  hostIsBigEndian )
     ||( This->order == dcrudByteOrder_BIG_ENDIAN    && !hostIsBigEndian ))
   {
      byte *   p   = (byte *)&value;
      byte     ton[8];
      unsigned i;
      for( i = 0U; i < 4U; ++i ) {
         ton[  i] = p[7-i];
         ton[7-i] = p[  i];
      }
      memcpy( This->bytes + This->position, ton, 8 );
   }
   else {
      memcpy( This->bytes + This->position, &value, 8 );
   }
   This->position += 8;
}

void dcrudByteBuffer_putBuffer( ioByteBuffer * This, ioByteBuffer * source ) {
   unsigned count = source->limit - source->position;
   memcpy( This->bytes + This->position, source->bytes + source->position, count );
   source->position += count;
   This  ->position += count;
}
