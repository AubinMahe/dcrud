#include <dcrud/Shareable.h>

typedef struct dcrudShareableImpl_s {

   dcrudSerializable           serializable;
   dcrudGUID                   id;
   dcrudShareable_setF         set;
   dcrudShareable_serializeF   serialize;
   dcrudShareable_unserializeF unserialize;

} dcrudShareableImpl;

void dcrudShareable_init(
   dcrudSerializable           serializable,
   dcrudShareable *            baseInSerializable,
   dcrudGUID                   id,
   dcrudShareable_setF         set,
   dcrudShareable_serializeF   serialize,
   dcrudShareable_unserializeF unserialize )
{
   dcrudShareableImpl * This = (dcrudShareableImpl *)malloc( sizeof( dcrudShareableImpl ));
   This->serializable = serializable;
   This->id           = id;
   This->set          = set;
   This->serialize    = serialize;
   This->unserialize  = unserialize;
   *baseInSerializable = (dcrudShareable)This;
}

dcrudGUID dcrudShareable_getId( dcrudShareable self ) {
   dcrudShareableImpl * This = (dcrudShareableImpl *)self;
   return This->id;
}

ioError dcrudShareable_serialize( dcrudShareable self, ioByteBuffer target ) {
   dcrudShareableImpl * This = (dcrudShareableImpl *)self;
   return This->serialize( This->serializable, target );
}

ioError dcrudShareable_unserialize( dcrudShareable self, ioByteBuffer source ) {
   dcrudShareableImpl * This = (dcrudShareableImpl *)self;
   return This->unserialize( This->serializable, source );
}
