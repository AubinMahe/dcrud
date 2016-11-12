#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <io/ByteBuffer.h>
#include <coll/ForeachFunction.h>

UTIL_ADT( dcrudClassID );

typedef enum Type_e {

   dcrudTYPE_NULL,
   dcrudTYPE_BOOLEAN,
   dcrudTYPE_BYTE,
   dcrudTYPE_CHAR,
   dcrudTYPE_SHORT,
   dcrudTYPE_UNSIGNED_SHORT,
   dcrudTYPE_INTEGER,
   dcrudTYPE_UNSIGNED_INTEGER,
   dcrudTYPE_LONG,
   dcrudTYPE_UNSIGNED_LONG,
   dcrudTYPE_FLOAT,
   dcrudTYPE_DOUBLE,
   dcrudTYPE_STRING,
   dcrudTYPE_CLASS_ID,
   dcrudTYPE_GUID,
   dcrudTYPE_SHAREABLE,

   dcrudLAST_TYPE

} dcrudType;

utilStatus dcrudClassID_resolve      ( /* */ dcrudClassID * This, byte package1, byte package2, byte package3, byte clazz );
utilStatus dcrudClassID_delete       ( /* */ dcrudClassID * This );
utilStatus dcrudClassID_get          ( const dcrudClassID   This, byte * pckg1, byte * pckg2, byte * pckg3, byte * clazz );
utilStatus dcrudClassID_unserialize  ( /* */ dcrudClassID * This, ioByteBuffer source );
utilStatus dcrudClassID_serialize    ( const dcrudClassID   This, ioByteBuffer target );
utilStatus dcrudClassID_serializeType( dcrudType type, ioByteBuffer target );
utilStatus dcrudClassID_toString     ( const dcrudClassID   This, char * target, size_t targetSize );
utilStatus dcrudClassID_getType      ( const dcrudClassID   This, dcrudType * result );
utilStatus dcrudClassID_printMapPair ( collForeach * context );
utilStatus dcrudClassID_done         ( void );
int        dcrudClassID_compareTo    ( const dcrudClassID * left, const dcrudClassID * right );

#ifdef __cplusplus
}
#endif
