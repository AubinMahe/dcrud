#include <dcrud/GUID.h>
#include <stdio.h>
#include <string.h>

typedef struct dcrudGUIDImpl_s {

   char topic[1024];
   int  classId;
   int  instance;

} dcrudGUIDImpl;

dcrudGUID dcrudGUID_init( const char * topic, int classId ) {
   dcrudGUIDImpl * This = (dcrudGUIDImpl *)malloc( sizeof(dcrudGUIDImpl));
   strncpy( This->topic, topic, sizeof( This->topic ));
   This->classId  = classId;
   This->instance = 0;
   return (dcrudGUID)This;
}

ioError dcrudGUID_unserialize( ioByteBuffer source, dcrudGUID * target ) {
   dcrudGUIDImpl * This = (dcrudGUIDImpl *)malloc( sizeof(dcrudGUIDImpl));
   ioError ioStatus = ioByteBuffer_getString( source,  This->topic, sizeof( This->topic ));
   unsigned v;
   if( ioError_NO_ERROR == ioStatus ) {
      ioStatus = ioByteBuffer_getInt( source, &v );
      This->classId = (int)v;
   }
   if( ioError_NO_ERROR == ioStatus ) {
      ioStatus = ioByteBuffer_getInt( source, &v );
      This->instance = (int)v;
   }
   if( ioError_NO_ERROR == ioStatus ) {
      *target = (dcrudGUID)This;
   }
   fprintf( stderr, "unable to deserialize a GUID: %s\n", ioErrorMessages[ioStatus] );
   return ioStatus;
}

void dcrudGUID_setInstance( const dcrudGUID self, int instance ) {
   dcrudGUIDImpl * This = (dcrudGUIDImpl *)self;
   This->instance = instance;
}

ioError dcrudGUID_serialize( const dcrudGUID self, ioByteBuffer target ) {
   dcrudGUIDImpl * This = (dcrudGUIDImpl *)self;
   ioError ioStatus = ioByteBuffer_putString( target, This->topic );
   if( ioError_NO_ERROR == ioStatus ) {
      ioStatus = ioByteBuffer_putInt( target, (unsigned)This->classId );
   }
   if( ioError_NO_ERROR == ioStatus ) {
      ioStatus = ioByteBuffer_putInt( target, (unsigned)This->instance );
   }
   if( ioError_NO_ERROR != ioStatus ) {
      fprintf( stderr, "unable to serialize a GUID: %s\n", ioErrorMessages[ioStatus] );
   }
   return ioStatus;
}

int dcrudGUID_getClassId( const dcrudGUID self ) {
   dcrudGUIDImpl * This = (dcrudGUIDImpl *)self;
   return This->classId;
}

int dcrudGUID_getInstanceId( const dcrudGUID self ) {
   dcrudGUIDImpl * This = (dcrudGUIDImpl *)self;
   return This->instance;
}

bool dcrudGUID_isShared( const dcrudGUID self ) {
   dcrudGUIDImpl * This = (dcrudGUIDImpl *)self;
   return This->instance != 0;
}

bool dcrudGUID_toString( const dcrudGUID self, char * target, size_t targetSize ) {
   dcrudGUIDImpl * This = (dcrudGUIDImpl *)self;
   int ret =
      snprintf( target, targetSize, "%s-%04X-%04X",
         This->topic, This->classId, This->instance );
   return ret > 0 && ret < (int)targetSize;
}

int dcrudGUID_compareTo( const dcrudGUID * l, const dcrudGUID * r ) {
   dcrudGUIDImpl ** left  = (dcrudGUIDImpl **)l;
   dcrudGUIDImpl ** right = (dcrudGUIDImpl **)r;
   int diff = (*left)->classId - (*right)->classId;
   if( diff == 0 ) {
      diff = (*left)->instance - (*right)->instance;
   }
   return diff;
}
