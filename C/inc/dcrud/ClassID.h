#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <io/ByteBuffer.h>
#include <coll/ForeachFunction.h>

UTIL_ADT( dcrudClassID );

typedef enum Type_e {

   dcrudTYPE_NULL,
   dcrudTYPE_BYTE,
   dcrudTYPE_BOOLEAN,
   dcrudTYPE_SHORT,
   dcrudTYPE_INTEGER,
   dcrudTYPE_LONG,
   dcrudTYPE_FLOAT,
   dcrudTYPE_DOUBLE,
   dcrudTYPE_STRING,
   dcrudTYPE_CLASS_ID,
   dcrudTYPE_GUID,
   dcrudTYPE_CALL_MODE,
   dcrudTYPE_QUEUE_INDEX,
   dcrudTYPE_SHAREABLE,

   dcrudLAST_TYPE

} dcrudType;

dcrudClassID dcrudClassID_new          ( byte package1, byte package2, byte package3, byte clazz );
void         dcrudClassID_delete       ( dcrudClassID * target );
void         dcrudClassID_get          ( const dcrudClassID This, byte * pckg1, byte * pckg2, byte * pckg3, byte * clazz );
ioStatus     dcrudClassID_unserialize  ( ioByteBuffer source, dcrudClassID * target );
ioStatus     dcrudClassID_serialize    ( const dcrudClassID This, ioByteBuffer target );
ioStatus     dcrudClassID_serializeType( dcrudType          type, ioByteBuffer target );
bool         dcrudClassID_toString     ( const dcrudClassID This, char * target, size_t targetSize );
int          dcrudClassID_compareTo    ( const dcrudClassID * left, const dcrudClassID * right );
dcrudType    dcrudClassID_getType      ( const dcrudClassID This );
bool         dcrudClassID_printMapPair ( collForeach * context );

#ifdef __cplusplus
}
#endif
