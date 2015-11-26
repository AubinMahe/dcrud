#include "Shareable_private.h"
#include "GUID_private.h"
#include "Network.h"

void dcrudShareable_delete( dcrudShareable * This ) {
   free( *This );
   *This = NULL;
}

dcrudGUID dcrudShareable_getGUID( dcrudShareable self ) {
   dcrudShareableImpl * This = (dcrudShareableImpl *)self;
   return This->id;
}

dcrudClassID dcrudShareable_getClassID( dcrudShareable self ) {
   dcrudShareableImpl * This = (dcrudShareableImpl *)self;
   return This->classID;
}

int dcrudShareable_compareTo( dcrudShareable * left, dcrudShareable * right ) {
   return (int)( *left - *right );
}

dcrudSerializable dcrudShareable_getUserData( dcrudShareable self ) {
   dcrudShareableImpl * This = (dcrudShareableImpl *)self;
   return This + 1;
}

dcrudShareable dcrudShareable_getShareable( dcrudSerializable user ) {
   dcrudShareableImpl * next = (dcrudShareableImpl *)user;
   return (dcrudShareable)(next - 1);
}
