#include "GUID_private.h"
#include <stdio.h>
#include <string.h>

dcrudGUID dcrudGUID_new() {
   dcrudGUIDImpl * This = (dcrudGUIDImpl *)malloc( sizeof( dcrudGUIDImpl ));
   memset( This, 0, sizeof( dcrudGUIDImpl ));
   return (dcrudGUID)This;
}

void dcrudGUID_delete( dcrudGUID * self ) {
   dcrudGUIDImpl * This = (dcrudGUIDImpl *)*self;
   free( This );
   *self = NULL;
}

ioStatus dcrudGUID_unserialize( ioByteBuffer source, dcrudGUID * target ) {
   dcrudGUIDImpl * This     = (dcrudGUIDImpl *)dcrudGUID_new();
   ioStatus        ioStatus = IO_STATUS_NO_ERROR;
   if( IO_STATUS_NO_ERROR == ioStatus ) {
      ioStatus = ioByteBuffer_getShort( source, &This->publisher );
   }
   if( IO_STATUS_NO_ERROR == ioStatus ) {
      ioStatus = ioByteBuffer_getByte( source, &This->cache );
   }
   if( IO_STATUS_NO_ERROR == ioStatus ) {
      ioStatus = ioByteBuffer_getInt( source, &This->instance );
   }
   if( IO_STATUS_NO_ERROR == ioStatus ) {
      *target = (dcrudGUID)This;
   }
   else {
      *target = NULL;
   }
   if( IO_STATUS_NO_ERROR != ioStatus ) {
      fprintf( stderr, "unable to deserialize a GUID: %s\n", ioStatusMessages[ioStatus] );
   }
   return ioStatus;
}

ioStatus dcrudGUID_serialize( const dcrudGUID self, ioByteBuffer target ) {
   dcrudGUIDImpl * This     = (dcrudGUIDImpl *)self;
   ioStatus        ioStatus = IO_STATUS_NO_ERROR;
   if( IO_STATUS_NO_ERROR == ioStatus ) {
      ioStatus = ioByteBuffer_putShort( target, This->publisher );
   }
   if( IO_STATUS_NO_ERROR == ioStatus ) {
      ioStatus = ioByteBuffer_putByte( target, This->cache );
   }
   if( IO_STATUS_NO_ERROR == ioStatus ) {
      ioStatus = ioByteBuffer_putInt( target, (unsigned)This->instance );
   }
   if( IO_STATUS_NO_ERROR != ioStatus ) {
      fprintf( stderr, "unable to serialize a GUID: %s\n", ioStatusMessages[ioStatus] );
   }
   return ioStatus;
}

bool dcrudGUID_isShared( const dcrudGUID self ) {
   dcrudGUIDImpl * This  = (dcrudGUIDImpl *)self;
   return This->instance > 0;
}

void dcrudGUID_set( dcrudGUID self, const dcrudGUID id ) {
   dcrudGUIDImpl * This  = (dcrudGUIDImpl *)self;
   dcrudGUIDImpl * right = (dcrudGUIDImpl *)id;
   This->publisher = right->publisher;
   This->cache     = right->cache;
   This->instance  = right->instance;
}

void dcrudGUID_init( dcrudGUID self, unsigned short publisher, byte cache, unsigned int instance ) {
   dcrudGUIDImpl * This  = (dcrudGUIDImpl *)self;
   This->publisher = publisher;
   This->cache     = cache;
   This->instance  = instance;
}

bool dcrudGUID_toString( const dcrudGUID self, char * target, size_t targetSize ) {
   dcrudGUIDImpl * This = (dcrudGUIDImpl *)self;
   int             ret  =
      snprintf( target, targetSize, "Instance-%02X-%02X-%04X",
         This->publisher, This->cache, This->instance );
   return ret > 0 && ret < (int)targetSize;
}

int dcrudGUID_compareTo( const dcrudGUID * l, const dcrudGUID * r ) {
   dcrudGUIDImpl ** left  = (dcrudGUIDImpl **)l;
   dcrudGUIDImpl ** right = (dcrudGUIDImpl **)r;
   int diff = 0;
   if( diff == 0 ) {
      diff =       (*left)->publisher - (*right)->publisher;
   }
   if( diff == 0 ) {
      diff =       (*left)->cache     - (*right)->cache;
   }
   if( diff == 0 ) {
      diff = (int)((*left)->instance  - (*right)->instance );
   }
   return diff;
}
