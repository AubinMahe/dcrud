#include <io/ByteBuffer.h>
#include <util/check.h>

#include <stdio.h>

typedef struct ioByteBufferImpl_s {

   ioByteOrder order;
   size_t      position;
   size_t      limit;
   size_t      capacity;
   size_t      mark;
   byte *      bytes;

} ioByteBufferImpl;

static bool init            = true;
static bool hostIsBigEndian = false;

ioByteBuffer ioByteBuffer_new( size_t capacity ) {
   ioByteBufferImpl * This = (ioByteBufferImpl *)malloc( sizeof( ioByteBufferImpl ));
   if( init ) {
      init = false;
      hostIsBigEndian = ( htonl(1) == 1 );
   }
   This->order    = ioByteOrder_BIG_ENDIAN;
   This->position = 0;
   This->limit    = capacity;
   This->capacity = capacity;
   This->mark     = capacity + 1;
   This->bytes    = (byte*)malloc( capacity );
   return (ioByteBuffer)This;
}

void ioByteBuffer_delete( ioByteBuffer * target ) {
   free( *target );
   *target = NULL;
}

void ioByteBuffer_clear( ioByteBuffer self ) {
   ioByteBufferImpl * This = (ioByteBufferImpl *)self;
   This->position = 0;
   This->limit    = This->capacity;
}

void ioByteBuffer_mark( ioByteBuffer self ) {
   ioByteBufferImpl * This = (ioByteBufferImpl *)self;
   This->mark = This->position;
}

ioStatus ioByteBuffer_reset( ioByteBuffer self ) {
   ioByteBufferImpl * This   = (ioByteBufferImpl *)self;
   ioStatus           status = IO_STATUS_NO_ERROR;
   if( This->mark < This->limit ) {
      This->position = This->mark;
      This->mark     = This->capacity + 1;
   }
   else {
      status = IO_STATUS_NO_MARK;
   }
   return status;
}

void ioByteBuffer_flip( ioByteBuffer self ) {
   ioByteBufferImpl * This = (ioByteBufferImpl *)self;
   This->limit    = This->position;
   This->position = 0;
}

size_t ioByteBuffer_getPosition( ioByteBuffer self ) {
   ioByteBufferImpl * This = (ioByteBufferImpl *)self;
   return This->position;
}

ioStatus ioByteBuffer_setPosition( ioByteBuffer self, size_t position ) {
   ioByteBufferImpl * This   = (ioByteBufferImpl *)self;
   ioStatus           status = IO_STATUS_NO_ERROR;
   if( position < This->limit ) {
      This->position = position;
   }
   else {
      status = IO_STATUS_OVERFLOW;
   }
   return status;
}

size_t ioByteBuffer_getLimit( ioByteBuffer self ) {
   ioByteBufferImpl * This = (ioByteBufferImpl *)self;
   return This->limit;
}

size_t ioByteBuffer_remaining( ioByteBuffer self ) {
   ioByteBufferImpl * This = (ioByteBufferImpl *)self;
   return This->limit - This->position;
}

ioStatus ioByteBuffer_put( ioByteBuffer self, const byte * src, size_t from, size_t to ) {
   ioByteBufferImpl * This = (ioByteBufferImpl *)self;
   const size_t count = to - from;
   if( This->position + count > This->limit ) {
      return IO_STATUS_OVERFLOW;
   }
   memcpy( This->bytes + This->position, src + from, count );
   This->position += count;
   return IO_STATUS_NO_ERROR;
}

ioStatus ioByteBuffer_get( ioByteBuffer self, byte * target, size_t from, size_t to ) {
   ioByteBufferImpl * This = (ioByteBufferImpl *)self;
   const size_t count = to - from;
   if( This->position + count > This->limit ) {
      return IO_STATUS_UNDERFLOW;
   }
   memcpy( target+from, This->bytes + This->position, count );
   This->position += count;
   return IO_STATUS_NO_ERROR;
}

ioStatus ioByteBuffer_putByte( ioByteBuffer self, byte value ) {
   ioByteBufferImpl * This   = (ioByteBufferImpl *)self;
   if( This->position + 1 > This->limit ) {
      return IO_STATUS_OVERFLOW;
   }
   This->bytes[This->position] = value;
   This->position += 1;
   return IO_STATUS_NO_ERROR;
}

ioStatus ioByteBuffer_getByte( ioByteBuffer self, byte * target ) {
   ioByteBufferImpl * This   = (ioByteBufferImpl *)self;
   if( This->position + 1 > This->limit ) {
      return IO_STATUS_UNDERFLOW;
   }
   *target = This->bytes[This->position];
   This->position += 1;
   return IO_STATUS_NO_ERROR;
}

/* Endianness solution:
 * http://stackoverflow.com/questions/2182002/convert-big-endian-to-little-endian-in-c-without-using-provided-func
 */

ioStatus ioByteBuffer_putShort( ioByteBuffer self, unsigned short value ) {
   ioByteBufferImpl * This = (ioByteBufferImpl *)self;
   if( This->position + sizeof( short ) > This->limit ) {
      return IO_STATUS_OVERFLOW;
   }
   if( ( This->order == ioByteOrder_LITTLE_ENDIAN &&  hostIsBigEndian )
     ||( This->order == ioByteOrder_BIG_ENDIAN    && !hostIsBigEndian ))
   {
      value = (( value & 0x00FF ) >> 8 )|(unsigned short)( value & 0xFF00 << 8 );
   }
   memcpy( This->bytes + This->position, &value, sizeof( short ));
   This->position += sizeof( short );
   return IO_STATUS_NO_ERROR;
}

ioStatus ioByteBuffer_getShort( ioByteBuffer self, unsigned short * target ) {
   ioByteBufferImpl * This = (ioByteBufferImpl *)self;
   unsigned short     value;
   if( This->position + sizeof( short ) > This->limit ) {
      return IO_STATUS_UNDERFLOW;
   }
   memcpy( &value, This->bytes + This->position, sizeof( short ));
   if( ( This->order == ioByteOrder_LITTLE_ENDIAN &&  hostIsBigEndian )
     ||( This->order == ioByteOrder_BIG_ENDIAN    && !hostIsBigEndian ))
   {
      value = (( value & 0x00FF ) >> 8 )|(unsigned short)( value & 0xFF00 << 8 );
   }
   *target = value;
   This->position += sizeof( short );
   return IO_STATUS_NO_ERROR;
}

ioStatus ioByteBuffer_putInt( ioByteBuffer self, unsigned int value ) {
   ioByteBufferImpl * This = (ioByteBufferImpl *)self;
   if( This->position + sizeof( int ) > This->limit ) {
      return IO_STATUS_OVERFLOW;
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
   return IO_STATUS_NO_ERROR;
}

ioStatus ioByteBuffer_putIntAt ( ioByteBuffer self, unsigned int value, unsigned int index ) {
   ioByteBufferImpl * This = (ioByteBufferImpl *)self;
   if( index + sizeof( int ) > This->limit ) {
      return IO_STATUS_OVERFLOW;
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
   return IO_STATUS_NO_ERROR;
}

ioStatus ioByteBuffer_getInt( ioByteBuffer self, unsigned int * target ) {
   ioByteBufferImpl * This = (ioByteBufferImpl *)self;
   unsigned int       value;
   if( This->position + sizeof( int ) > This->limit ) {
      return IO_STATUS_UNDERFLOW;
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
   return IO_STATUS_NO_ERROR;
}

ioStatus ioByteBuffer_putLong( ioByteBuffer self, uint64_t value ) {
   ioByteBufferImpl * This = (ioByteBufferImpl *)self;
   if( This->position + sizeof( uint64_t ) > This->limit ) {
      return IO_STATUS_OVERFLOW;
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
   return IO_STATUS_NO_ERROR;
}

ioStatus ioByteBuffer_getLong( ioByteBuffer self, uint64_t * target ) {
   ioByteBufferImpl * This = (ioByteBufferImpl *)self;
   uint64_t           value;
   if( This->position + sizeof( uint64_t ) > This->limit ) {
      return IO_STATUS_UNDERFLOW;
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
   return IO_STATUS_NO_ERROR;
}

ioStatus ioByteBuffer_putDouble( ioByteBuffer self, double value ) {
   return ioByteBuffer_putLong( self, *(uint64_t*)(void*)&value );
}

ioStatus ioByteBuffer_getDouble( ioByteBuffer self, double * target ) {
   return ioByteBuffer_getLong( self, (uint64_t *)target );
}

ioStatus ioByteBuffer_putString( ioByteBuffer This, const char * source ) {
   unsigned int len    = (unsigned int)strlen( source );
   ioStatus      status = ioByteBuffer_putInt( This, len );
   if( status == IO_STATUS_NO_ERROR ) {
      status = ioByteBuffer_put( This, (const byte *)source, 0U, len );
   }
   return status;
}

ioStatus ioByteBuffer_getString( ioByteBuffer self, char * dest, size_t size ) {
   ioByteBufferImpl * This   = (ioByteBufferImpl *)self;
   unsigned int       len    = 0U;
   ioStatus            status = ioByteBuffer_getInt( self, &len );
   if( status == IO_STATUS_NO_ERROR ) {
      if( len > size ) {
         status = ioByteBuffer_get( self, (byte *)dest, 0, size );
         This->position += len-size;
      }
      else if( size > len ) {
         status = IO_STATUS_UNDERFLOW;
      }
      else {
         status = ioByteBuffer_get( self, (byte *)dest, 0, len );
      }
   }
   return status;
}

ioStatus ioByteBuffer_putBuffer( ioByteBuffer self, ioByteBuffer other ) {
   ioByteBufferImpl * This   = (ioByteBufferImpl *)self;
   ioByteBufferImpl * source = (ioByteBufferImpl *)other;
   size_t             count  = source->limit - source->position;
   if( This->position + count > This->limit ) {
      return IO_STATUS_OVERFLOW;
   }
   memcpy( This->bytes + This->position, source->bytes + source->position, count );
   source->position += count;
   This  ->position += count;
   return IO_STATUS_NO_ERROR;
}

ioStatus ioByteBuffer_send( ioByteBuffer self, SOCKET sckt, struct sockaddr_in * trgt ) {
   ioByteBufferImpl * This   = (ioByteBufferImpl *)self;
   ioStatus           retVal = IO_STATUS_NO_ERROR;
   size_t             len    = This->limit - This->position;
   const char *       buffer = (const char *)( This->bytes + This->position );
   ssize_t            count  =
      sendto( sckt, buffer, len, 0, (struct sockaddr *)trgt, sizeof( struct sockaddr_in ));
   if( count < 0 || ((int)len) != count ) {
      retVal = IO_STATUS_SENDTO_FAILED;
   }
   else {
      This->position += (unsigned)count;
   }
   return retVal;
}

ioStatus ioByteBuffer_receive( ioByteBuffer self, SOCKET sckt ) {
   ioByteBufferImpl * This   = (ioByteBufferImpl *)self;
   ioStatus           retVal = IO_STATUS_NO_ERROR;
   size_t             max    = This->limit-This->position;
   void *             buffer = This->bytes+This->position;
   ssize_t            count  = recv( sckt, buffer, max, 0 );
   if( count < 0 ) {
      retVal = IO_STATUS_RECV_FAILED;
   }
   else {
      This->position += (unsigned)count;
   }
   return retVal;
}

byte * ioByteBuffer_getBytes( ioByteBuffer self ) {
   ioByteBufferImpl * This = (ioByteBufferImpl *)self;
   return This->bytes;
}

ioByteBuffer ioByteBuffer_copy( ioByteBuffer self, size_t length ) {
   ioByteBufferImpl * This = (ioByteBufferImpl *)self;
   ioByteBufferImpl * copy = (ioByteBufferImpl *)ioByteBuffer_new( length );
   memcpy( copy->bytes, This->bytes + This->position, length );
   return (ioByteBuffer)copy;
}
