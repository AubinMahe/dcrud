#include "ClassID_private.h"
#include "magic.h"
#include "poolSizes.h"

#include <coll/Map.h>
#include <util/Pool.h>

#include <stdio.h>
#include <string.h>

UTIL_DEFINE_SAFE_CAST( dcrudClassID     )
UTIL_POOL_DECLARE    ( dcrudClassIDImpl )

static dcrudClassIDImpl * allClasses[dcrudGUIDImpl_POOL_SIZE];
static unsigned           allClassesCount;

utilStatus dcrudClassID_resolve(
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
      dcrudClassIDImpl   key = {
         dcrudClassIDImplMAGIC,
         package1,
         package2,
         package3,
         clazz
      };
      dcrudClassIDImpl *  This   = NULL;
      dcrudClassIDImpl *  keyPtr = &key;
      dcrudClassIDImpl ** cached = bsearch(
         &keyPtr, allClasses, allClassesCount, sizeof( dcrudClassIDImpl ),
         (__compar_fn_t)dcrudClassID_compareTo );
      if( cached ) {
         *self = (dcrudClassID)*cached;
         return status;
      }
      UTIL_ALLOCATE_ADT( dcrudClassID, self, This );
      if( UTIL_STATUS_NO_ERROR == status ) {
         This->package_1 = package1;
         This->package_2 = package2;
         This->package_3 = package3;
         This->clazz     = clazz;
         allClasses[allClassesCount++] = This;
         qsort(
            allClasses,
            allClassesCount,
            sizeof( dcrudClassIDImpl ),
            (__compar_fn_t)dcrudClassID_compareTo );
      }
   }
   return status;
}

utilStatus dcrudClassID_done( void ) {
   unsigned i;
   for( i = 0U; i < allClassesCount; ++i ) {
      dcrudClassIDImpl * classID = allClasses[i];
#ifdef STATIC_ALLOCATION
      utilPool_release( &dcrudClassIDImplPool, &classID );
#else
      free( classID );
#endif
   }
   allClassesCount = 0U;
   return UTIL_STATUS_NO_ERROR;
}

utilStatus dcrudClassID_delete( dcrudClassID * self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
//   UTIL_RELEASE( dcrudClassIDImpl );
   *self = NULL;
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
         status = dcrudClassID_resolve( self, p1, p2, p3, c );
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
   diff = left->magic     - right->magic;
   if( diff ) {
      return diff;
   }
   diff = left->package_1 - right->package_1;
   if( diff ) {
      return diff;
   }
   diff = left->package_2 - right->package_2;
   if( diff ) {
      return diff;
   }
   diff = left->package_3 - right->package_3;
   if( diff ) {
      return diff;
   }
   diff = left->clazz     - right->clazz;
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
   if( target ) {
      fprintf( target, "%s => %s\n", name, buffer );
   }
   else {
      printf( "%s => %s\n", name, buffer );
   }
   return status;
}
