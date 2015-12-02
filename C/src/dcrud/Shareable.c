#include "Shareable_private.h"
#include "GUID_private.h"
#include "ParticipantImpl.h"

dcrudShareable dcrudShareable_new( dcrudShareableClass * meta, dcrudClassID classID ) {
   dcrudShareableImpl * This =
      UTIL_CAST( dcrudShareableImpl *, malloc( sizeof( dcrudShareableImpl ) + meta->size ));
   This->classID     = classID;
   This->id          = dcrudGUID_new();
   This->serialize   = meta->serialize;
   This->unserialize = meta->unserialize;
   return UTIL_CAST( dcrudShareable, This );
}

void dcrudShareable_delete( dcrudShareable * self ) {
   dcrudShareableImpl * This = UTIL_CAST( dcrudShareableImpl *, *self );
   if( This ) {
      free( This->id );
      free( This );
      *self = NULL;
   }
}

dcrudGUID dcrudShareable_getGUID( dcrudShareable self ) {
   dcrudShareableImpl * This = UTIL_CAST( dcrudShareableImpl *, self );
   return This->id;
}

dcrudClassID dcrudShareable_getClassID( dcrudShareable self ) {
   dcrudShareableImpl * This = UTIL_CAST( dcrudShareableImpl *, self );
   return This->classID;
}

int dcrudShareable_compareTo( dcrudShareable * left, dcrudShareable * right ) {
   return UTIL_CAST( int, *left - *right );
}

dcrudShareableData dcrudShareable_getUserData( dcrudShareable self ) {
   dcrudShareableImpl * This = UTIL_CAST( dcrudShareableImpl *, self );
   return This + 1;
}

dcrudShareable dcrudShareable_getShareable( dcrudShareableData user ) {
   dcrudShareableImpl * next = UTIL_CAST(dcrudShareableImpl *, user );
   return UTIL_CAST( dcrudShareable, next - 1 );
}
