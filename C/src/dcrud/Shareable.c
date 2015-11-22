#include <dcrud/Shareable.h>
#include "GUID_private.h"

typedef struct dcrudShareableImpl_s {

   dcrudSerializable           serializable;
   dcrudGUID                   id;
   dcrudClassID                clazz;
   dcrudShareable_setF         set;
   dcrudShareable_serializeF   serialize;
   dcrudShareable_unserializeF unserialize;

} dcrudShareableImpl;

void dcrudShareable_init(
   dcrudSerializable           serializable,
   dcrudShareable *            baseInSerializable,
   dcrudShareable_setF         set,
   dcrudShareable_serializeF   serialize,
   dcrudShareable_unserializeF unserialize )
{
   dcrudShareableImpl * This = (dcrudShareableImpl *)malloc( sizeof( dcrudShareableImpl ));
   This->serializable = serializable;
   This->id           = (dcrudGUID)dcrudGUID_init();
   This->set          = set;
   This->serialize    = serialize;
   This->unserialize  = unserialize;
   *baseInSerializable = (dcrudShareable)This;
}

dcrudGUID dcrudShareable_getGUID( dcrudShareable self ) {
   dcrudShareableImpl * This = (dcrudShareableImpl *)self;
   return This->id;
}

dcrudClassID dcrudShareable_getClassID( dcrudShareable self ) {
   dcrudShareableImpl * This = (dcrudShareableImpl *)self;
   return This->clazz;
}

ioStatus dcrudShareable_serialize( dcrudShareable self, ioByteBuffer target ) {
   dcrudShareableImpl * This = (dcrudShareableImpl *)self;
   return This->serialize( This->serializable, target );
}

ioStatus dcrudShareable_unserialize( dcrudShareable self, ioByteBuffer source ) {
   dcrudShareableImpl * This = (dcrudShareableImpl *)self;
   return This->unserialize( This->serializable, source );
}

int dcrudShareable_compareTo( dcrudShareable * left, dcrudShareable * right ) {
   return (int)( *left - *right );
}
