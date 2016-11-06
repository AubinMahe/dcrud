#include "ClassID_private.h"
#include "magic.h"
#include "poolSizes.h"

#include <coll/Map.h>
#include <util/Pool.h>

#include <stdio.h>
#include <string.h>

UTIL_DEFINE_SAFE_CAST( dcrudClassID     )
UTIL_POOL_DECLARE    ( dcrudClassIDImpl )

utilStatus dcrudClassID_new(
   dcrudClassID * self,
   byte           package1,
   byte           package2,
   byte           package3,
   byte           clazz     )
{
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( self == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudClassIDImpl * This = NULL;
      UTIL_ALLOCATE_ADT( dcrudClassID, self, This );
      if( UTIL_STATUS_NO_ERROR == status ) {
         This->package_1 = package1;
         This->package_2 = package2;
         This->package_3 = package3;
         This->clazz     = clazz;
      }
   }
   return status;
}

utilStatus dcrudClassID_delete( dcrudClassID * self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   UTIL_RELEASE( dcrudClassIDImpl );
   return status;
}

utilStatus dcrudClassID_get( const dcrudClassID self, byte * pckg1, byte * pckg2, byte * pckg3, byte * clazz ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   dcrudClassIDImpl * This = dcrudClassID_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      *pckg1 = This->package_1;
      *pckg2 = This->package_2;
      *pckg3 = This->package_3;
      *clazz = This->clazz;
   }
   return status;
}

utilStatus dcrudClassID_unserialize( dcrudClassID * self, ioByteBuffer source ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( NULL == self ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      byte p1, p2, p3, c;
      /*     */ioByteBuffer_getByte( source, &p1 );
      /*     */ioByteBuffer_getByte( source, &p2 );
      /*     */ioByteBuffer_getByte( source, &p3 );
      status = ioByteBuffer_getByte( source, &c  );
      if( UTIL_STATUS_NO_ERROR == status ) {
         status = dcrudClassID_new( self, p1, p2, p3, c );
      }
      else {
         *self = NULL;
      }
   }
   return status;
}

utilStatus dcrudClassID_serialize( const dcrudClassID self, ioByteBuffer target ) {
   utilStatus         status = UTIL_STATUS_NO_ERROR;
   dcrudClassIDImpl * This   = dcrudClassID_safeCast( self, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      status = ioByteBuffer_putByte( target, This->package_1 );
   }
   if( UTIL_STATUS_NO_ERROR == status ) {
      status = ioByteBuffer_putByte( target, This->package_2 );
   }
   if( UTIL_STATUS_NO_ERROR == status ) {
      status = ioByteBuffer_putByte( target, This->package_3 );
   }
   if( UTIL_STATUS_NO_ERROR == status ) {
      status = ioByteBuffer_putByte( target, This->clazz );
   }
   return status;
}

utilStatus dcrudClassID_serializeType( dcrudType type, ioByteBuffer target ) {
   utilStatus status = ioByteBuffer_putByte( target, 0 );
   if( UTIL_STATUS_NO_ERROR == status ) {
      status = ioByteBuffer_putByte( target, 0 );
   }
   if( UTIL_STATUS_NO_ERROR == status ) {
      status = ioByteBuffer_putByte( target, 0 );
   }
   if( UTIL_STATUS_NO_ERROR == status ) {
      status = ioByteBuffer_putByte( target, type );
   }
   return status;
}

utilStatus dcrudClassID_toString( const dcrudClassID self, char * target, size_t targetSize ) {
   utilStatus         status = UTIL_STATUS_NO_ERROR;
   dcrudClassIDImpl * This   = dcrudClassID_safeCast( self, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      int ret = snprintf( target, targetSize, "Class-%02X-%02X-%02X-%02X",
         This->package_1, This->package_2, This->package_3, This->clazz );
      if( ret < 0 ) {
         status = UTIL_STATUS_STD_API_ERROR;
      }
      else if( ret >= (int)targetSize ) {
         status = UTIL_STATUS_OVERFLOW;
      }
   }
   return status;
}

int dcrudClassID_compareTo( const dcrudClassID * l, const dcrudClassID * r ) {
   dcrudClassIDImpl * left  = (dcrudClassIDImpl *)*l;
   dcrudClassIDImpl * right = (dcrudClassIDImpl *)*r;
   int                diff  = 0;

   if( left == NULL && right == NULL ) {
      return 0;
   }
   if( left == NULL ) {
      return -1;
   }
   if( right == NULL ) {
      return +1;
   }
   if( diff == 0 ) {
      diff = left->package_1 - right->package_1;
   }
   if( diff == 0 ) {
      diff = left->package_2 - right->package_2;
   }
   if( diff == 0 ) {
      diff = left->package_3 - right->package_3;
   }
   if( diff == 0 ) {
      diff = left->clazz     - right->clazz;
   }
   return diff;
}

utilStatus dcrudClassID_getType( const dcrudClassID self, dcrudType * result ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( NULL == result ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudClassIDImpl * This = dcrudClassID_safeCast( self, &status );
      if( UTIL_STATUS_NO_ERROR == status ) {
         if( This->package_1 != 0 || This->package_2 != 0 || This->package_3 != 0 ) {
            *result = dcrudLAST_TYPE;
         }
         else {
            *result = (dcrudType)This->clazz;
         }
      }
   }
   return status;
}

utilStatus dcrudClassID_printMapPair( collForeach * context ) {
   char         buffer[40];
   FILE *       target = (FILE *      )context->user;
   const char * name   = (const char *)context->key;
   dcrudClassID value  = (dcrudClassID)context->value;
   utilStatus   status = dcrudClassID_toString( value, buffer, sizeof( buffer ));
   fprintf( target, "%s => %s\n", name, buffer );
   return status;
}
