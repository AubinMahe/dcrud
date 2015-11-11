#include <dcrud/Shareable.h>

typedef struct dcrudShareableImpl_s {

   dcrudSerializable           serializable;
   int                         classId;
   dcrudGUID                   id;
   dcrudShareable_setF         set;
   dcrudShareable_serializeF   serialize;
   dcrudShareable_unserializeF unserialize;

} dcrudShareableImpl;

void dcrudShareable_init(
   dcrudSerializable           serializable,
   dcrudShareable *            baseInSerializable,
   int                         classId,
   dcrudShareable_setF         set,
   dcrudShareable_serializeF   serialize,
   dcrudShareable_unserializeF unserialize )
{
   dcrudShareableImpl * This = (dcrudShareableImpl *)malloc( sizeof( dcrudShareableImpl ));
   This->serializable = serializable;
   This->classId      = classId;
   This->id           = NULL;
   This->set          = set;
   This->serialize    = serialize;
   This->unserialize  = unserialize;
   *baseInSerializable = (dcrudShareable)This;
}

int dcrudShareable_getClassId( dcrudShareable self ) {
   dcrudShareableImpl * This = (dcrudShareableImpl *)self;
   return This->classId;
}

dcrudGUID dcrudShareable_getId( dcrudShareable self ) {
   dcrudShareableImpl * This = (dcrudShareableImpl *)self;
   return This->id;
}

void dcrudShareable_setId( dcrudShareable self, dcrudGUID id ) {
   dcrudShareableImpl * This = (dcrudShareableImpl *)self;
   This->id = id;
}

void dcrudShareable_serialize( dcrudShareable self, ioByteBuffer * target ) {
   dcrudShareableImpl * This = (dcrudShareableImpl *)self;
   This->serialize( This->serializable, target );
}

void dcrudShareable_unserialize( dcrudShareable self, ioByteBuffer * source ) {
   dcrudShareableImpl * This = (dcrudShareableImpl *)self;
   This->unserialize( This->serializable, source );
}
