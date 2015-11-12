#include <io/ByteBuffer.h>
#include <util/check.h>

#include <stdio.h>

typedef struct ioByteBufferImpl_s {

   ioByteOrder order;
   size_t      position;
   size_t      limit;
   size_t      capacity;
   byte *      bytes;

} ioByteBufferImpl;

static bool init            = true;
static bool hostIsBigEndian = false;

ioByteBuffer ioByteBuffer_allocate( size_t capacity ) {
   ioByteBufferImpl * This = (ioByteBufferImpl *)malloc( sizeof( ioByteBufferImpl ));
   if( init ) {
      init = false;
      hostIsBigEndian = ( htonl(1) == 1 );
   }
   This->order    = ioByteOrder_BIG_ENDIAN;
   This->position = 0;
   This->limit    = capacity;
   This->capacity = capacity;
   This->bytes    = (byte*)malloc( capacity );
   return (ioByteBuffer)This;
}

void ioByteBuffer_clear( ioByteBuffer self ) {
   ioByteBufferImpl * This = (ioByteBufferImpl *)self;
   This->position = 0;
   This->limit    = This->capacity;
}

void ioByteBuffer_flip( ioByteBuffer self ) {
   ioByteBufferImpl * This = (ioByteBufferImpl *)self;
   This->limit    = This->position;
   This->position = 0;
}

size_t ioByteBuffer_position( ioByteBuffer self ) {
   ioByteBufferImpl * This = (ioByteBufferImpl *)self;
   return This->position;
}

size_t ioByteBuffer_limit( ioByteBuffer self ) {
   ioByteBufferImpl * This = (ioByteBufferImpl *)self;
   return This->limit;
}

size_t ioByteBuffer_remaining( ioByteBuffer self ) {
   ioByteBufferImpl * This = (ioByteBufferImpl *)self;
   return This->limit - This->position;
}

ioError ioByteBuffer_put( ioByteBuffer self, const byte * src, size_t from, size_t to ) {
   ioByteBufferImpl * This = (ioByteBufferImpl *)self;
   const size_t count = to - from;
   if( This->position + count > This->limit ) {
      return ioError_OVERFLOW;
   }
   memcpy( This->bytes + This->position, src + from, count );
   This->position += count;
   return ioError_NO_ERROR;
}

ioError ioByteBuffer_get( ioByteBuffer self, byte * target, size_t from, size_t to ) {
   ioByteBufferImpl * This = (ioByteBufferImpl *)self;
   const size_t count = to - from;
   if( This->position + count > This->limit ) {
      return ioError_UNDERFLOW;
   }
   memcpy( target+from, This->bytes + This->position, count );
   This->position += count;
   return ioError_NO_ERROR;
}

/* Endianness solution:
 * http://stackoverflow.com/questions/2182002/convert-big-endian-to-little-endian-in-c-without-using-provided-func
 */

ioError ioByteBuffer_putShort( ioByteBuffer self, unsigned short value ) {
   ioByteBufferImpl * This = (ioByteBufferImpl *)self;
   if( This->position + sizeof( short ) > This->limit ) {
      return ioError_OVERFLOW;
   }
   if( ( This->order == ioByteOrder_LITTLE_ENDIAN &&  hostIsBigEndian )
     ||( This->order == ioByteOrder_BIG_ENDIAN    && !hostIsBigEndian ))
   {
      value = (( value & 0x00FF ) >> 8 )|(unsigned short)( value & 0xFF00 << 8 );
   }
   memcpy( This->bytes + This->position, &value, sizeof( short ));
   This->position += sizeof( short );
   return ioError_NO_ERROR;
}

ioError ioByteBuffer_getShort( ioByteBuffer self, unsigned short * target ) {
   ioByteBufferImpl * This = (ioByteBufferImpl *)self;
   unsigned short     value;
   if( This->position + sizeof( short ) > This->limit ) {
      return ioError_UNDERFLOW;
   }
   memcpy( &value, This->bytes + This->position, sizeof( short ));
   if( ( This->order == ioByteOrder_LITTLE_ENDIAN &&  hostIsBigEndian )
     ||( This->order == ioByteOrder_BIG_ENDIAN    && !hostIsBigEndian ))
   {
      value = (( value & 0x00FF ) >> 8 )|(unsigned short)( value & 0xFF00 << 8 );
   }
   *target = value;
   This->position += sizeof( short );
   return ioError_NO_ERROR;
}

ioError ioByteBuffer_putInt( ioByteBuffer self, unsigned int value ) {
   ioByteBufferImpl * This = (ioByteBufferImpl *)self;
   if( This->position + sizeof( int ) > This->limit ) {
      return ioError_OVERFLOW;
   }
   if( ( This->order == ioByteOrder_LITTLE_ENDIAN &&  hostIsBigEndian )
     ||( This->order == ioByteOrder_BIG_ENDIAN    && !hostIsBigEndian ))
   {
      value = (( value & 0xFF000000) >> 24 )
             |(( value & 0x00FF0000) >>  8 )
             |(( value & 0x0000FF00) <<  8 )
             |(  value               << 24 );
   }
   memcpy( This->bytes + This->position, &value, sizeof( int ));
   This->position += sizeof( int );
   return ioError_NO_ERROR;
}

ioError ioByteBuffer_putIntAt ( ioByteBuffer self, unsigned int value, unsigned int index ) {
   ioByteBufferImpl * This = (ioByteBufferImpl *)self;
   if( index + sizeof( int ) > This->limit ) {
      return ioError_OVERFLOW;
   }
   if( ( This->order == ioByteOrder_LITTLE_ENDIAN &&  hostIsBigEndian )
     ||( This->order == ioByteOrder_BIG_ENDIAN    && !hostIsBigEndian ))
   {
      value = (( value & 0xFF000000) >> 24 )
             |(( value & 0x00FF0000) >>  8 )
             |(( value & 0x0000FF00) <<  8 )
             |(  value               << 24 );
   }
   memcpy( This->bytes + index, &value, sizeof( int ));
   return ioError_NO_ERROR;
}

ioError ioByteBuffer_getInt( ioByteBuffer self, unsigned int * target ) {
   ioByteBufferImpl * This = (ioByteBufferImpl *)self;
   unsigned int       value;
   if( This->position + sizeof( int ) > This->limit ) {
      return ioError_UNDERFLOW;
   }
   memcpy( &value, This->bytes + This->position, sizeof( int ));
   if( ( This->order == ioByteOrder_LITTLE_ENDIAN &&  hostIsBigEndian )
     ||( This->order == ioByteOrder_BIG_ENDIAN    && !hostIsBigEndian ))
   {
      value = (( value & 0xFF000000) >> 24 )
             |(( value & 0x00FF0000) >>  8 )
             |(( value & 0x0000FF00) <<  8 )
             |(  value               << 24 );
   }
   *target = value;
   This->position += sizeof( int );
   return ioError_NO_ERROR;
}

ioError ioByteBuffer_putLong( ioByteBuffer self, uint64_t value ) {
   ioByteBufferImpl * This = (ioByteBufferImpl *)self;
   if( This->position + sizeof( uint64_t ) > This->limit ) {
      return ioError_OVERFLOW;
   }
   if( ( This->order == ioByteOrder_LITTLE_ENDIAN &&  hostIsBigEndian )
     ||( This->order == ioByteOrder_BIG_ENDIAN    && !hostIsBigEndian ))
   {
      value = (( value & 0xFF00000000000000LL) >> 56 )
             |(( value & 0x00FF000000000000LL) >> 40 )
             |(( value & 0x0000FF0000000000LL) >> 24 )
             |(( value & 0x000000FF00000000LL) >>  8 )
             |(( value & 0x00000000FF000000LL) <<  8 )
             |(( value & 0x0000000000FF0000LL) << 24 )
             |(( value & 0x000000000000FF00LL) << 40 )
             |(  value                         << 56 );
   }
   memcpy( This->bytes + This->position, &value, sizeof( uint64_t ));
   This->position += sizeof( uint64_t );
   return ioError_NO_ERROR;
}

ioError ioByteBuffer_getLong( ioByteBuffer self, uint64_t * target ) {
   ioByteBufferImpl * This = (ioByteBufferImpl *)self;
   uint64_t           value;
   if( This->position + sizeof( uint64_t ) > This->limit ) {
      return ioError_UNDERFLOW;
   }
   memcpy( &value, This->bytes + This->position, sizeof( int64_t ));
   if( ( This->order == ioByteOrder_LITTLE_ENDIAN &&  hostIsBigEndian )
     ||( This->order == ioByteOrder_BIG_ENDIAN    && !hostIsBigEndian ))
   {
      value = (( value & 0xFF00000000000000LL) >> 56 )
             |(( value & 0x00FF000000000000LL) >> 40 )
             |(( value & 0x0000FF0000000000LL) >> 24 )
             |(( value & 0x000000FF00000000LL) >>  8 )
             |(( value & 0x00000000FF000000LL) <<  8 )
             |(( value & 0x0000000000FF0000LL) << 24 )
             |(( value & 0x000000000000FF00LL) << 40 )
             |(  value                         << 56 );
   }
   *target = value;
   This->position += sizeof( uint64_t );
   return ioError_NO_ERROR;
}

ioError ioByteBuffer_putDouble( ioByteBuffer self, double value ) {
   return ioByteBuffer_putLong( self, *(uint64_t*)(void*)&value );
}

ioError ioByteBuffer_getDouble( ioByteBuffer self, double * target ) {
   return ioByteBuffer_getLong( self, (uint64_t *)target );
}

ioError ioByteBuffer_putString( ioByteBuffer This, const char * source ) {
   unsigned int len    = (unsigned int)strlen( source );
   ioError      status = ioByteBuffer_putInt( This, len );
   if( status == ioError_NO_ERROR ) {
      status = ioByteBuffer_put( This, (const byte *)source, 0U, len );
   }
   return status;
}

ioError ioByteBuffer_getString( ioByteBuffer self, char * dest, size_t size ) {
   ioByteBufferImpl * This   = (ioByteBufferImpl *)self;
   unsigned int       len    = 0U;
   ioError            status = ioByteBuffer_getInt( self, &len );
   if( status == ioError_NO_ERROR ) {
      if( len > size ) {
         status = ioByteBuffer_get( self, (byte *)dest, 0, size );
         This->position += len-size;
      }
      else if( size > len ) {
         status = ioError_UNDERFLOW;
      }
      else {
         status = ioByteBuffer_get( self, (byte *)dest, 0, len );
      }
   }
   return status;
}

ioError ioByteBuffer_putBuffer( ioByteBuffer self, ioByteBuffer other ) {
   ioByteBufferImpl * This   = (ioByteBufferImpl *)self;
   ioByteBufferImpl * source = (ioByteBufferImpl *)other;
   size_t             count  = source->limit - source->position;
   if( This->position + count > This->limit ) {
      return ioError_OVERFLOW;
   }
   memcpy( This->bytes + This->position, source->bytes + source->position, count );
   source->position += count;
   This  ->position += count;
   return ioError_NO_ERROR;
}

ioError ioByteBuffer_sendTo(
   ioByteBuffer         self,
   SOCKET               sckt,
   struct sockaddr_in * trgt,
   int *                count )
{
   ioByteBufferImpl * This = (ioByteBufferImpl *)self;
   size_t             len  = This->limit;
   *count = (int)
      sendto(
         sckt,
         (const char *)This->bytes,
         len,
         0,
         (struct sockaddr *)trgt,
         sizeof( struct sockaddr_in ));
   check(((int)len) == *count, __FILE__, __LINE__, "sendto %d bytes sent to %s:%d",
      *count, inet_ntoa( trgt->sin_addr ), ntohs( trgt->sin_port ));
   return ioError_NO_ERROR;
}
