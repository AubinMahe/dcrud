#include <dcrud/GUID.h>
#include <stdio.h>

void dcrudGUID_init( dcrudGUID * This, int source, int instance ) {
   This->source   = source;
   This->instance = instance;
}

bool dcrudGUID_matchSource( const dcrudGUID * This, int source ) {
   return This->source == source;
}

bool dcrudGUID_toString( const dcrudGUID * This, char * target, size_t targetSize ) {
   int ret = snprintf( target, targetSize, "%04X-%04X", This->source, This->instance );
   return ret > 0 && ret < (int)targetSize;
}

int dcrudGUID_compareTo( const dcrudGUID * left, const dcrudGUID * right ) {
   int diff = left->source - right->source;
   if( diff == 0 ) {
      diff = left->instance - right->instance;
   }
   return diff;
}
