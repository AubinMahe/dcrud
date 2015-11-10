#include <dcrud/GUID.h>
#include <stdio.h>

typedef struct dcrudGUIDImpl_s {

   int source;
   int instance;

} dcrudGUIDImpl;

dcrudGUID dcrudGUID_init( int source, int instance ) {
   dcrudGUIDImpl * This = (dcrudGUIDImpl *)malloc( sizeof(dcrudGUIDImpl));
   This->source   = source;
   This->instance = instance;
   return (dcrudGUID)This;
}

bool dcrudGUID_isValid( dcrudGUID self ) {
   dcrudGUIDImpl * This = (dcrudGUIDImpl *)self;
   return This->instance != 0;
}

bool dcrudGUID_matchSource( const dcrudGUID self, int source ) {
   dcrudGUIDImpl * This = (dcrudGUIDImpl *)self;
   return This->source == source;
}

bool dcrudGUID_toString( const dcrudGUID self, char * target, size_t targetSize ) {
   dcrudGUIDImpl * This = (dcrudGUIDImpl *)self;
   int ret = snprintf( target, targetSize, "%04X-%04X", This->source, This->instance );
   return ret > 0 && ret < (int)targetSize;
}

void dcrudGUID_serialize( const dcrudGUID self, ioByteBuffer * target ) {
   dcrudGUIDImpl * This = (dcrudGUIDImpl *)self;
   dcrudByteBuffer_putInt( target, (unsigned)This->source );
   dcrudByteBuffer_putInt( target, (unsigned)This->instance );
}

int dcrudGUID_compareTo( const dcrudGUID * l, const dcrudGUID * r ) {
   dcrudGUIDImpl ** left  = (dcrudGUIDImpl **)l;
   dcrudGUIDImpl ** right = (dcrudGUIDImpl **)r;
   int diff = (*left)->source - (*right)->source;
   if( diff == 0 ) {
      diff = (*left)->instance - (*right)->instance;
   }
   return diff;
}
