#include <io/ByteBuffer.h>
#include "poolSizes.h"
#include "magic.h"

#include <util/Dump.h>
#include <util/Pool.h>

#ifdef _MSC_VER
#  define ssize_t int
#endif

typedef struct ioByteBufferImpl_s {

   unsigned    magic;
   ioByteOrder order;
   size_t      position;
   size_t      limit;
   size_t      capacity;
   size_t      mark;
   byte *      bytes;

} ioByteBufferImpl;

UTIL_DEFINE_SAFE_CAST( ioByteBuffer     )
UTIL_POOL_DECLARE    ( ioByteBufferImpl )

static bool init            = true;
static bool hostIsBigEndian = false;

utilStatus ioByteBuffer_wrap( ioByteBuffer * self, size_t capacity, byte * array ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   ioByteBufferImpl * This = NULL;
   UTIL_ALLOCATE_ADT( ioByteBuffer, self, This );
   if( UTIL_STATUS_NO_ERROR == status ) {
      This->order    = ioByteOrder_BIG_ENDIAN;
      This->position = 0;
      This->limit    = capacity;
      This->capacity = capacity;
      This->mark     = capacity + 1;
      This->bytes    = array;
      memset( This->bytes, 0, capacity );
   }
   if( init ) {
      init            = false;
      hostIsBigEndian = ( htonl(1) == 1 );
   }
   return status;
}

typedef byte ioChunk___256[  256];
typedef byte ioChunk___512[  512];
typedef byte ioChunk__1024[ 1024];
typedef byte ioChunk__2048[ 2048];
typedef byte ioChunk__4096[ 4096];
typedef byte ioChunk__8192[ 8192];
typedef byte ioChunk_16384[16384];
typedef byte ioChunk_32768[32768];
typedef byte ioChunk_65536[65536];

#define ioChunk___256_POOL_SIZE 4000
#define ioChunk___512_POOL_SIZE 2000
#define ioChunk__1024_POOL_SIZE 1000
#define ioChunk__2048_POOL_SIZE  500
#define ioChunk__4096_POOL_SIZE  250
#define ioChunk__8192_POOL_SIZE  125
#define ioChunk_16384_POOL_SIZE   50
#define ioChunk_32768_POOL_SIZE   25
#define ioChunk_65536_POOL_SIZE   10

UTIL_POOL_DECLARE( ioChunk___256 )
UTIL_POOL_DECLARE( ioChunk___512 )
UTIL_POOL_DECLARE( ioChunk__1024 )
UTIL_POOL_DECLARE( ioChunk__2048 )
UTIL_POOL_DECLARE( ioChunk__4096 )
UTIL_POOL_DECLARE( ioChunk__8192 )
UTIL_POOL_DECLARE( ioChunk_16384 )
UTIL_POOL_DECLARE( ioChunk_32768 )
UTIL_POOL_DECLARE( ioChunk_65536 )

utilStatus ioByteBuffer_new( ioByteBuffer * This, size_t capacity ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
#ifdef STATIC_ALLOCATION
   byte * buffer = NULL;
   if( init ) {
      UTIL_POOL_INIT( ioChunk___256 );
      UTIL_POOL_INIT( ioChunk___512 );
      UTIL_POOL_INIT( ioChunk__1024 );
      UTIL_POOL_INIT( ioChunk__2048 );
      UTIL_POOL_INIT( ioChunk__4096 );
      UTIL_POOL_INIT( ioChunk__8192 );
      UTIL_POOL_INIT( ioChunk_16384 );
      UTIL_POOL_INIT( ioChunk_32768 );
      UTIL_POOL_INIT( ioChunk_65536 );
   }
   if( false ) {}
   else if( capacity <=   256U ) status = utilPool_reserve( &ioChunk___256Pool, &buffer );
   else if( capacity <=   512U ) status = utilPool_reserve( &ioChunk___512Pool, &buffer );
   else if( capacity <=  1024U ) status = utilPool_reserve( &ioChunk__1024Pool, &buffer );
   else if( capacity <=  2048U ) status = utilPool_reserve( &ioChunk__2048Pool, &buffer );
   else if( capacity <=  4096U ) status = utilPool_reserve( &ioChunk__4096Pool, &buffer );
   else if( capacity <=  8192U ) status = utilPool_reserve( &ioChunk__8192Pool, &buffer );
   else if( capacity <= 16384U ) status = utilPool_reserve( &ioChunk_16384Pool, &buffer );
   else if( capacity <= 32768U ) status = utilPool_reserve( &ioChunk_32768Pool, &buffer );
   else if( capacity <= 65536U ) status = utilPool_reserve( &ioChunk_65536Pool, &buffer );
   else {
      status = UTIL_STATUS_TOO_MANY;
   }
#else
   byte * buffer = (byte *)malloc( capacity );
   if( buffer == NULL ) {
      status = UTIL_STATUS_TOO_MANY;
   }
#endif
   if( UTIL_STATUS_NO_ERROR == status ) {
      status = ioByteBuffer_wrap( This, capacity, buffer );
   }
   return status;
}

utilStatus ioByteBuffer_delete( ioByteBuffer * self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( self == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      ioByteBufferImpl * This = ioByteBuffer_safeCast( *self, &status );
      if( UTIL_STATUS_NO_ERROR == status ) {
#ifdef STATIC_ALLOCATION
         if( false ) {}
         else if( This->capacity <=   256U ) utilPool_release( &ioChunk___256Pool, &This->bytes );
         else if( This->capacity <=   512U ) utilPool_release( &ioChunk___512Pool, &This->bytes );
         else if( This->capacity <=  1024U ) utilPool_release( &ioChunk__1024Pool, &This->bytes );
         else if( This->capacity <=  2048U ) utilPool_release( &ioChunk__2048Pool, &This->bytes );
         else if( This->capacity <=  4096U ) utilPool_release( &ioChunk__4096Pool, &This->bytes );
         else if( This->capacity <=  8192U ) utilPool_release( &ioChunk__8192Pool, &This->bytes );
         else if( This->capacity <= 16384U ) utilPool_release( &ioChunk_16384Pool, &This->bytes );
         else if( This->capacity <= 32768U ) utilPool_release( &ioChunk_32768Pool, &This->bytes );
         else if( This->capacity <= 65536U ) utilPool_release( &ioChunk_65536Pool, &This->bytes );
#else
         free( This->bytes );
#endif
         UTIL_RELEASE( ioByteBufferImpl )
      }
   }
   return status;
}

utilStatus ioByteBuffer_copy( ioByteBuffer self, ioByteBuffer * to, size_t length ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   ioByteBufferImpl * This = ioByteBuffer_safeCast( self, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      status = ioByteBuffer_new( to, length );
      if( UTIL_STATUS_NO_ERROR == status ) {
         ioByteBufferImpl * target = (ioByteBufferImpl *)*to;
         memcpy( target->bytes, This->bytes + This->position, length );
      }
   }
   return status;
}

utilStatus ioByteBuffer_array( ioByteBuffer self, byte ** array ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( array == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      ioByteBufferImpl * This = ioByteBuffer_safeCast( self, &status );
      if( UTIL_STATUS_NO_ERROR == status ) {
         *array = This->bytes;
      }
   }
   return status;
}

utilStatus ioByteBuffer_clear( ioByteBuffer self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   ioByteBufferImpl * This = ioByteBuffer_safeCast( self, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      This->position = 0;
      This->limit    = This->capacity;
   }
   return status;
}

utilStatus ioByteBuffer_mark( ioByteBuffer self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   ioByteBufferImpl * This = ioByteBuffer_safeCast( self, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      This->mark = This->position;
   }
   return status;
}

utilStatus ioByteBuffer_reset( ioByteBuffer self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   ioByteBufferImpl * This = ioByteBuffer_safeCast( self, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      if( This->mark < This->limit ) {
         This->position = This->mark;
         This->mark     = This->capacity + 1;
      }
      else {
         status = UTIL_STATUS_ILLEGAL_STATE;
      }
   }
   return status;
}

utilStatus ioByteBuffer_flip( ioByteBuffer self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   ioByteBufferImpl * This = ioByteBuffer_safeCast( self, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      This->limit    = This->position;
      This->position = 0;
   }
   return status;
}

utilStatus ioByteBuffer_getPosition( ioByteBuffer self, size_t * position ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( position ) {
      ioByteBufferImpl * This = ioByteBuffer_safeCast( self, &status );
      if( UTIL_STATUS_NO_ERROR == status ) {
         *position = This->position;
      }
   }
   else {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   return status;
}

utilStatus ioByteBuffer_setPosition( ioByteBuffer self, size_t position ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   ioByteBufferImpl * This = ioByteBuffer_safeCast( self, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      if( position <= This->limit ) {
         This->position = position;
      }
      else {
         status = UTIL_STATUS_OVERFLOW;
      }
   }
   return status;
}

utilStatus ioByteBuffer_getLimit( ioByteBuffer self, size_t * limit ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( limit ) {
      ioByteBufferImpl * This = ioByteBuffer_safeCast( self, &status );
      if( UTIL_STATUS_NO_ERROR == status ) {
         *limit = This->limit;
      }
   }
   else {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   return status;
}

utilStatus ioByteBuffer_remaining( ioByteBuffer self, size_t * remaining ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( remaining ) {
      ioByteBufferImpl * This = ioByteBuffer_safeCast( self, &status );
      if( UTIL_STATUS_NO_ERROR == status ) {
         *remaining = This->limit - This->position;
      }
   }
   else {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   return status;
}

utilStatus ioByteBuffer_put( ioByteBuffer self, const byte * src, size_t from, size_t to ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   ioByteBufferImpl * This = ioByteBuffer_safeCast( self, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      const size_t count = to - from;
      if( This->position + count > This->limit ) {
         return UTIL_STATUS_OVERFLOW;
      }
      memcpy( This->bytes + This->position, src + from, count );
      This->position += count;
   }
   return status;
}

utilStatus ioByteBuffer_get( ioByteBuffer self, byte * target, size_t from, size_t to ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   ioByteBufferImpl * This = ioByteBuffer_safeCast( self, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      const size_t count = to - from;
      if( This->position + count > This->limit ) {
         return UTIL_STATUS_UNDERFLOW;
      }
      memcpy( target+from, This->bytes + This->position, count );
      This->position += count;
   }
   return status;
}

utilStatus ioByteBuffer_putByte( ioByteBuffer self, byte value ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   ioByteBufferImpl * This = ioByteBuffer_safeCast( self, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      if( This->position + 1 > This->limit ) {
         return UTIL_STATUS_OVERFLOW;
      }
      This->bytes[This->position] = value;
      This->position += 1;
   }
   return status;
}

utilStatus ioByteBuffer_getByte( ioByteBuffer self, byte * target ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   ioByteBufferImpl * This = ioByteBuffer_safeCast( self, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      if( This->position + 1 > This->limit ) {
         return UTIL_STATUS_UNDERFLOW;
      }
      *target = This->bytes[This->position];
      This->position += 1;
   }
   return status;
}

/* Endianness solution:
 * http://stackoverflow.com/questions/2182002/convert-big-endian-to-little-endian-in-c-without-using-provided-func
 */

utilStatus ioByteBuffer_putShort( ioByteBuffer self, short value ) {
   return ioByteBuffer_putUShort( self, (unsigned short)value );
}

utilStatus ioByteBuffer_getShort( ioByteBuffer self, short * target ) {
   return ioByteBuffer_getUShort( self, (unsigned short *)target );
}

utilStatus ioByteBuffer_putUShort( ioByteBuffer self, unsigned short value ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   ioByteBufferImpl * This = ioByteBuffer_safeCast( self, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      if( This->position + sizeof( unsigned short ) > This->limit ) {
         return UTIL_STATUS_OVERFLOW;
      }
      if( ( This->order == ioByteOrder_LITTLE_ENDIAN &&  hostIsBigEndian )
        ||( This->order == ioByteOrder_BIG_ENDIAN    && !hostIsBigEndian ))
      {
         value = (unsigned short)((( value & 0xFF00 ) >> 8 )|(( value & 0x00FF ) << 8 ));
      }
      memcpy( This->bytes + This->position, &value, sizeof( unsigned short ));
      This->position += sizeof( unsigned short );
   }
   return status;
}

utilStatus ioByteBuffer_getUShort( ioByteBuffer self, unsigned short * target ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   ioByteBufferImpl * This = ioByteBuffer_safeCast( self, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      unsigned short     value;
      if( This->position + sizeof( unsigned short ) > This->limit ) {
         return UTIL_STATUS_UNDERFLOW;
      }
      memcpy( &value, This->bytes + This->position, sizeof( unsigned short ));
      if( ( This->order == ioByteOrder_LITTLE_ENDIAN &&  hostIsBigEndian )
        ||( This->order == ioByteOrder_BIG_ENDIAN    && !hostIsBigEndian ))
      {
         value = (unsigned short)(
                (( value & 0xFF00 ) >> 8 )
               |(( value & 0x00FF ) << 8 ));
      }
      *target = value;
      This->position += sizeof( short );
   }
   return status;
}

utilStatus ioByteBuffer_putInt( ioByteBuffer self, int value ) {
   return ioByteBuffer_putUInt( self, (unsigned int)value );
}

utilStatus ioByteBuffer_getInt( ioByteBuffer self, int * target ) {
   return ioByteBuffer_getUInt( self, (unsigned int *)target );
}

utilStatus ioByteBuffer_putUInt( ioByteBuffer self, unsigned int value ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   ioByteBufferImpl * This = ioByteBuffer_safeCast( self, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      if( This->position + sizeof( unsigned int ) > This->limit ) {
         return UTIL_STATUS_OVERFLOW;
      }
      if( ( This->order == ioByteOrder_LITTLE_ENDIAN &&  hostIsBigEndian )
        ||( This->order == ioByteOrder_BIG_ENDIAN    && !hostIsBigEndian ))
      {
         value = (( value & 0xFF000000 ) >> 24 )
                |(( value & 0x00FF0000 ) >>  8 )
                |(( value & 0x0000FF00 ) <<  8 )
                |(  value                << 24 );
      }
      memcpy( This->bytes + This->position, &value, sizeof( unsigned int ));
      This->position += sizeof( unsigned int );
   }
   return status;
}

utilStatus ioByteBuffer_putUIntAt( ioByteBuffer self, unsigned int value, size_t index ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   ioByteBufferImpl * This = ioByteBuffer_safeCast( self, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      if( index + sizeof( unsigned int ) > This->limit ) {
         return UTIL_STATUS_OVERFLOW;
      }
      if( ( This->order == ioByteOrder_LITTLE_ENDIAN &&  hostIsBigEndian )
        ||( This->order == ioByteOrder_BIG_ENDIAN    && !hostIsBigEndian ))
      {
         value = (( value & 0xFF000000 ) >> 24 )
                |(( value & 0x00FF0000 ) >>  8 )
                |(( value & 0x0000FF00 ) <<  8 )
                |(  value                << 24 );
      }
      memcpy( This->bytes + index, &value, sizeof( unsigned int ));
   }
   return status;
}

utilStatus ioByteBuffer_getUInt( ioByteBuffer self, unsigned int * target ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   ioByteBufferImpl * This = ioByteBuffer_safeCast( self, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      unsigned int       value;
      if( This->position + sizeof( unsigned int ) > This->limit ) {
         return UTIL_STATUS_UNDERFLOW;
      }
      memcpy( &value, This->bytes + This->position, sizeof( int ));
      if( ( This->order == ioByteOrder_LITTLE_ENDIAN &&  hostIsBigEndian )
        ||( This->order == ioByteOrder_BIG_ENDIAN    && !hostIsBigEndian ))
      {
         value = (( value & 0xFF000000 ) >> 24 )
                |(( value & 0x00FF0000 ) >>  8 )
                |(( value & 0x0000FF00 ) <<  8 )
                |(  value                << 24 );
      }
      *target = value;
      This->position += sizeof( unsigned int );
   }
   return status;
}

utilStatus ioByteBuffer_putLong( ioByteBuffer self, int64_t value ) {
   return ioByteBuffer_putULong( self, (uint64_t)value );
}

utilStatus ioByteBuffer_getLong( ioByteBuffer self, int64_t * target ) {
   return ioByteBuffer_getULong( self, (uint64_t *)target );
}

utilStatus ioByteBuffer_putULong( ioByteBuffer self, uint64_t value ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   ioByteBufferImpl * This = ioByteBuffer_safeCast( self, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      if( This->position + sizeof( uint64_t ) > This->limit ) {
         return UTIL_STATUS_OVERFLOW;
      }
      if( ( This->order == ioByteOrder_LITTLE_ENDIAN &&  hostIsBigEndian )
        ||( This->order == ioByteOrder_BIG_ENDIAN    && !hostIsBigEndian ))
      {
         value = (( value & 0xFF00000000000000LL ) >> 56 )
                |(( value & 0x00FF000000000000LL ) >> 40 )
                |(( value & 0x0000FF0000000000LL ) >> 24 )
                |(( value & 0x000000FF00000000LL ) >>  8 )
                |(( value & 0x00000000FF000000LL ) <<  8 )
                |(( value & 0x0000000000FF0000LL ) << 24 )
                |(( value & 0x000000000000FF00LL ) << 40 )
                |(  value                          << 56 );
      }
      memcpy( This->bytes + This->position, &value, sizeof( uint64_t ));
      This->position += sizeof( uint64_t );
   }
   return status;
}

utilStatus ioByteBuffer_getULong( ioByteBuffer self, uint64_t * target ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   ioByteBufferImpl * This = ioByteBuffer_safeCast( self, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      uint64_t value;
      if( This->position + sizeof( uint64_t ) > This->limit ) {
         return UTIL_STATUS_UNDERFLOW;
      }
      memcpy( &value, This->bytes + This->position, sizeof( int64_t ));
      if( ( This->order == ioByteOrder_LITTLE_ENDIAN &&  hostIsBigEndian )
        ||( This->order == ioByteOrder_BIG_ENDIAN    && !hostIsBigEndian ))
      {
         value = (( value & 0xFF00000000000000LL ) >> 56 )
                |(( value & 0x00FF000000000000LL ) >> 40 )
                |(( value & 0x0000FF0000000000LL ) >> 24 )
                |(( value & 0x000000FF00000000LL ) >>  8 )
                |(( value & 0x00000000FF000000LL ) <<  8 )
                |(( value & 0x0000000000FF0000LL ) << 24 )
                |(( value & 0x000000000000FF00LL ) << 40 )
                |(  value                          << 56 );
      }
      *target = value;
      This->position += sizeof( uint64_t );
   }
   return status;
}

utilStatus ioByteBuffer_putFloat( ioByteBuffer self, float value ) {
   void * ptr = &value;
   return ioByteBuffer_putUInt( self, *(unsigned int*)ptr );
}

utilStatus ioByteBuffer_getFloat( ioByteBuffer self, float * target ) {
   return ioByteBuffer_getUInt( self, (unsigned int *)target );
}

utilStatus ioByteBuffer_putDouble( ioByteBuffer self, double value ) {
   void * ptr = &value;
   return ioByteBuffer_putULong( self, *(uint64_t*)ptr );
}

utilStatus ioByteBuffer_getDouble( ioByteBuffer self, double * target ) {
   return ioByteBuffer_getULong( self, (uint64_t *)target );
}

utilStatus ioByteBuffer_putString( ioByteBuffer This, const char * source ) {
   size_t     len    = strlen( source );
   utilStatus status = ioByteBuffer_putUInt( This, (unsigned int)len );
   if( UTIL_STATUS_NO_ERROR == status ) {
      status = ioByteBuffer_put( This, (const byte *)source, 0U, len );
   }
   return status;
}

utilStatus ioByteBuffer_getString( ioByteBuffer self, char * dest, size_t size ) {
   unsigned int   len    = 0U;
   utilStatus     status = ioByteBuffer_getUInt( self, &len );
   if( UTIL_STATUS_NO_ERROR == status ) {
      if( len < size ) {
         status = ioByteBuffer_get( self, (byte *)dest, 0, len );
         if( UTIL_STATUS_NO_ERROR == status ) {
            dest[len] = '\0';
         }
      }
      else if( size >= len ) {
         status = UTIL_STATUS_UNDERFLOW;
      }
   }
   return status;
}

utilStatus ioByteBuffer_putBuffer( ioByteBuffer self, ioByteBuffer other ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   ioByteBufferImpl * This = ioByteBuffer_safeCast( self, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      ioByteBufferImpl * source = ioByteBuffer_safeCast( other, &status );
      if( UTIL_STATUS_NO_ERROR == status ) {
         size_t count = source->limit - source->position;
         if( This->position + count > This->limit ) {
            return UTIL_STATUS_OVERFLOW;
         }
         memcpy( This->bytes + This->position, source->bytes + source->position, count );
         source->position += count;
         This  ->position += count;
      }
   }
   return status;
}

utilStatus ioByteBuffer_send( ioByteBuffer self, SOCKET sckt ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   ioByteBufferImpl * This = ioByteBuffer_safeCast( self, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      size_t       len    = This->limit - This->position;
      const char * buffer = (const char *)( This->bytes + This->position );
      ssize_t      count  = send( sckt, buffer, len, 0);
      if( count < 0 || ( len != (size_t)count )) {
         status = UTIL_STATUS_STD_API_ERROR;
      }
      else {
         This->position += (size_t)count;
      }
   }
   return status;
}

utilStatus ioByteBuffer_sendTo( ioByteBuffer self, SOCKET sckt, struct sockaddr_in * trgt ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   ioByteBufferImpl * This = ioByteBuffer_safeCast( self, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      size_t       len    = This->limit - This->position;
      const char * buffer = (const char *)( This->bytes + This->position );
      ssize_t      count  =
         sendto( sckt, buffer, len, 0, (struct sockaddr *)trgt, sizeof( struct sockaddr_in ));
      if( count < 0 || ( len != (size_t)count )) {
         status = UTIL_STATUS_STD_API_ERROR;
      }
      else {
         This->position += (size_t)count;
      }
   }
   return status;
}

utilStatus ioByteBuffer_receive( ioByteBuffer self, SOCKET sckt ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   ioByteBufferImpl * This = ioByteBuffer_safeCast( self, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      int       type;
      size_t    max    = This->limit-This->position;
      void *    buffer = This->bytes+This->position;
      socklen_t length = sizeof( type );
      ssize_t   count;
#ifdef WIN32
      getsockopt( sckt, SOL_SOCKET, SO_TYPE, (void *)&type, (int *)&length );
#else
      getsockopt( sckt, SOL_SOCKET, SO_TYPE, &type, &length );
#endif
      if( type == SOCK_STREAM ) {
         count = recv( sckt, buffer, max, 0 );
      }
      else {
         count = recvfrom( sckt, buffer, max, 0, NULL, NULL );
      }
      if( count < 0 ) {
         status = UTIL_STATUS_STD_API_ERROR;
      }
      else if( count == 0 ) {
         status = UTIL_STATUS_ILLEGAL_STATE;
      }
      else {
         This->position += (size_t)count;
      }
   }
   return status;
}

utilStatus ioByteBuffer_dump( ioByteBuffer self, FILE * target ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   ioByteBufferImpl * This = ioByteBuffer_safeCast( self, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      utilDump_range( target, This->bytes, 0, This->limit );
   }
   return status;
}
