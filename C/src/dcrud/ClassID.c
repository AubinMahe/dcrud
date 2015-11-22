#include "ClassID_private.h"
#include <stdio.h>
#include <string.h>

dcrudClassID dcrudClassID_init( byte package1, byte package2, byte package3, byte clazz ) {
   dcrudClassIDImpl * This = (dcrudClassIDImpl *)malloc( sizeof( dcrudClassIDImpl ));
   memset( This, 0, sizeof( dcrudClassIDImpl ));
   This->package_1 = package1;
   This->package_2 = package2;
   This->package_3 = package3;
   This->clazz     = clazz;
   return (dcrudClassID)This;
}

ioStatus dcrudClassID_unserialize( ioByteBuffer source, dcrudClassID * target ) {
   ioStatus           ioStatus = IO_STATUS_NO_ERROR;
   dcrudClassIDImpl * This     = (dcrudClassIDImpl *)dcrudClassID_init(0,0,0,0);
   if( IO_STATUS_NO_ERROR == ioStatus ) {
      ioStatus = ioByteBuffer_getByte( source, &This->package_1 );
   }
   if( IO_STATUS_NO_ERROR == ioStatus ) {
      ioStatus = ioByteBuffer_getByte( source, &This->package_2 );
   }
   if( IO_STATUS_NO_ERROR == ioStatus ) {
      ioStatus = ioByteBuffer_getByte( source, &This->package_3 );
   }
   if( IO_STATUS_NO_ERROR == ioStatus ) {
      ioStatus = ioByteBuffer_getByte( source, &This->clazz );
   }
   if( IO_STATUS_NO_ERROR == ioStatus ) {
      *target = (dcrudClassID)This;
   }
   else {
      *target = NULL;
   }
   if( IO_STATUS_NO_ERROR != ioStatus ) {
      fprintf( stderr, "unable to deserialize a GUID: %s\n", ioStatusMessages[ioStatus] );
   }
   return ioStatus;
}

ioStatus dcrudClassID_serialize( const dcrudClassID self, ioByteBuffer target ) {
   dcrudClassIDImpl * This     = (dcrudClassIDImpl *)self;
   ioStatus           ioStatus = IO_STATUS_NO_ERROR;
   if( IO_STATUS_NO_ERROR == ioStatus ) {
      ioStatus = ioByteBuffer_putByte( target, This->package_1 );
   }
   if( IO_STATUS_NO_ERROR == ioStatus ) {
      ioStatus = ioByteBuffer_putByte( target, This->package_2 );
   }
   if( IO_STATUS_NO_ERROR == ioStatus ) {
      ioStatus = ioByteBuffer_putByte( target, This->package_3 );
   }
   if( IO_STATUS_NO_ERROR == ioStatus ) {
      ioStatus = ioByteBuffer_putByte( target, This->clazz );
   }
   if( IO_STATUS_NO_ERROR != ioStatus ) {
      fprintf( stderr, "unable to serialize a GUID: %s\n", ioStatusMessages[ioStatus] );
   }
   return ioStatus;
}

bool dcrudClassID_toString( const dcrudClassID self, char * target, size_t targetSize ) {
   dcrudClassIDImpl * This = (dcrudClassIDImpl *)self;
   int                ret  =
      snprintf( target, targetSize, "Class-%02X-%02X-%02X-%02X",
         This->package_1, This->package_2, This->package_3, This->clazz );
   return ret > 0 && ret < (int)targetSize;
}

int dcrudClassID_compareTo( const dcrudClassID * l, const dcrudClassID * r ) {
   dcrudClassIDImpl ** left  = (dcrudClassIDImpl **)l;
   dcrudClassIDImpl ** right = (dcrudClassIDImpl **)r;
   int diff = 0;
   if( diff == 0 ) {
      diff = (*left)->package_1 - (*right)->package_1;
   }
   if( diff == 0 ) {
      diff = (*left)->package_2 - (*right)->package_2;
   }
   if( diff == 0 ) {
      diff = (*left)->package_3 - (*right)->package_3;
   }
   if( diff == 0 ) {
      diff = (*left)->clazz     - (*right)->clazz;
   }
   return diff;
}
