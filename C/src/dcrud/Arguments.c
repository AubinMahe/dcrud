#include <dcrud/Arguments.h>
#include "poolSizes.h"

#include <coll/Map.h>
#include <coll/Comparator.h>

#include <util/Pool.h>
#include <util/String.h>
#include <util/Trace.h>


#include "Shareable_private.h"
#include "GUID_private.h"
#include "magic.h"

typedef union dcrudArgumentsValue_u {

   dcrudType      Type;
   bool           Boolean;
   byte           Byte;
   char           Char;
   short          Short;
   unsigned short Ushort;
   int            Int;
   unsigned int   Uint;
   int64_t        Long;
   uint64_t       Ulong;
   float          Float;
   double         Double;
   char *         String;
   dcrudClassID   ClassID;
   dcrudGUID      GUID;
   dcrudShareable Shareable;

} dcrudArgumentsValue;

UTIL_POOL_DECLARE( dcrudArgumentsValue )

typedef struct dcrudArgumentsImpl_s {

   unsigned int    magic;
   dcrudCallMode   mode;
   dcrudQueueIndex queue;
   collMap         args;
   collMap         types;

} dcrudArgumentsImpl;

static dcrudType typNULL        = dcrudTYPE_NULL;
static dcrudType typBOOLEAN     = dcrudTYPE_BOOLEAN;
static dcrudType typBYTE        = dcrudTYPE_BYTE;
static dcrudType typCHAR        = dcrudTYPE_CHAR;
static dcrudType typSHORT       = dcrudTYPE_SHORT;
static dcrudType typUSHORT      = dcrudTYPE_UNSIGNED_SHORT;
static dcrudType typINTEGER     = dcrudTYPE_INTEGER;
static dcrudType typUINTEGER    = dcrudTYPE_UNSIGNED_INTEGER;
static dcrudType typLONG        = dcrudTYPE_LONG;
static dcrudType typULONG       = dcrudTYPE_UNSIGNED_LONG;
static dcrudType typFLOAT       = dcrudTYPE_FLOAT;
static dcrudType typDOUBLE      = dcrudTYPE_DOUBLE;
static dcrudType typSTRING      = dcrudTYPE_STRING;
static dcrudType typCLASS_ID    = dcrudTYPE_CLASS_ID;
static dcrudType typGUID        = dcrudTYPE_GUID;
static dcrudType typSHAREABLE   = dcrudTYPE_SHAREABLE;

UTIL_DEFINE_SAFE_CAST( dcrudArguments     )
UTIL_POOL_DECLARE    ( dcrudArgumentsImpl )

utilStatus dcrudArguments_new( dcrudArguments * self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( NULL == self ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudArgumentsImpl * This = NULL;
      UTIL_ALLOCATE_ADT( dcrudArguments, self, This  );
      if( UTIL_STATUS_NO_ERROR == status ) {
         status = collMap_new(&(This->args ), (collComparator)collStringCompare );
      }
      if( UTIL_STATUS_NO_ERROR == status ) {
         status = collMap_new(&(This->types), (collComparator)collStringCompare );
      }
      if( UTIL_STATUS_NO_ERROR == status ) {
         status = dcrudArguments_setMode((dcrudArguments)This, DCRUD_ASYNCHRONOUS_DEFERRED );
      }
      if( UTIL_STATUS_NO_ERROR == status ) {
         status = dcrudArguments_setQueue((dcrudArguments)This, DCRUD_DEFAULT_QUEUE );
      }
      if( UTIL_STATUS_NO_ERROR != status ) {
         collMap_delete(&(This->args));
         collMap_delete(&(This->types));
         free( This );
         *self = NULL;
      }
   }
   return status;
}

static utilStatus releaseValue( collForeach * context ) {
   utilStatus            status = UTIL_STATUS_NO_ERROR;
   dcrudArgumentsImpl *  This   = (dcrudArgumentsImpl * )context->user;
   dcrudArgumentsValue * value  = (dcrudArgumentsValue *)context->value;
   if( value ) {
      char *    key = (char *)context->key;
      dcrudType type;
      CHK(__FILE__,__LINE__,dcrudArguments_getType((dcrudArguments)This, key, &type ));
      if( typSTRING == type  ) {
         CHK(__FILE__,__LINE__,utilString_delete( &value->String ))
      }
      else if( typCLASS_ID == type  ) {
         CHK(__FILE__,__LINE__,dcrudClassID_delete( &value->ClassID ))
      }
      else if( typGUID == type  ) {
         CHK(__FILE__,__LINE__,dcrudGUID_delete( &value->GUID ))
      }
#ifdef STATIC_ALLOCATION
      utilPool_release( &dcrudArgumentsValuePool, &value );
#else
      free( value );
#endif
   }
   return status;
}

static utilStatus releaseKey( collForeach * context ) {
   char *     key    = (char *)context->key;
   utilStatus status = utilString_delete( &key );
   return status;
}

utilStatus dcrudArguments_delete( dcrudArguments * self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( self == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudArgumentsImpl * This = dcrudArguments_safeCast( *self, &status );
      if( status == UTIL_STATUS_NO_ERROR ) {
         collMap_foreach( This->args, releaseValue, This );
         collMap_foreach( This->args, releaseKey  , This );
         collMap_delete( &This->args );
         collMap_delete( &This->types );
         UTIL_RELEASE( dcrudArgumentsImpl );
      }
   }
   return status;
}

utilStatus dcrudArguments_clear( dcrudArguments self ) {
   utilStatus           status = UTIL_STATUS_NO_ERROR;
   dcrudArgumentsImpl * This   = dcrudArguments_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      collMap_foreach( This->args, releaseValue, This );
      status = collMap_clear( This->args );
   }
   return status;
}

utilStatus dcrudArguments_setMode( dcrudArguments self, dcrudCallMode value ) {
   utilStatus           status = UTIL_STATUS_NO_ERROR;
   dcrudArgumentsImpl * This   = dcrudArguments_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      This->mode = value;
   }
   return status;
}

utilStatus dcrudArguments_setQueue( dcrudArguments self, dcrudQueueIndex value ) {
   utilStatus           status = UTIL_STATUS_NO_ERROR;
   dcrudArgumentsImpl * This   = dcrudArguments_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      This->queue = value;
   }
   return status;
}

utilStatus dcrudArguments_isEmpty( dcrudArguments self, bool * result ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( NULL == result ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudArguments_safeCast( self, &status );
      if( status == UTIL_STATUS_NO_ERROR ) {
         unsigned count = 0U;
         status = dcrudArguments_getCount( self, &count );
         if( status == UTIL_STATUS_NO_ERROR ) {
            *result = ( count == 0 );
         }
      }
   }
   return status;
}

utilStatus dcrudArguments_getCount( dcrudArguments self, unsigned * result ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( NULL == result ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudArgumentsImpl * This = dcrudArguments_safeCast( self, &status );
      if( status == UTIL_STATUS_NO_ERROR ) {
         unsigned size = 0U;
         status  = collMap_size( This->args, &size );
         if( status == UTIL_STATUS_NO_ERROR ) {
            *result = size;
         }
      }
   }
   return status;
}

utilStatus dcrudArguments_getMode( dcrudArguments self, dcrudCallMode * result ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( NULL == result ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudArgumentsImpl * This   = dcrudArguments_safeCast( self, &status );
      if( status == UTIL_STATUS_NO_ERROR ) {
         *result = This->mode;
      }
   }
   return status;
}

utilStatus dcrudArguments_getQueue( dcrudArguments self, dcrudQueueIndex * result ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( NULL == result ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudArgumentsImpl * This = dcrudArguments_safeCast( self, &status );
      if( status == UTIL_STATUS_NO_ERROR ) {
         *result = This->queue;
      }
   }
   return status;
}

typedef struct Context_t {

   dcrudArgumentsImpl *  source;
   ioByteBuffer buffer;
   FILE *       file;

} Context;

static utilStatus serializePair( collForeach * context ) {
   Context *                   ctxt   = (Context *                  )context->user;
   const char *                key    = (const char *               )context->key;
   const dcrudArgumentsValue * value  = (const dcrudArgumentsValue *)context->value;
   dcrudArgumentsImpl *        source = ctxt->source;
   ioByteBuffer                target = ctxt->buffer;
   dcrudType                   type;
   utilStatus                  status;

   if( NULL == value ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      status = dcrudArguments_getType((dcrudArguments)source, key, &type );
      if( UTIL_STATUS_NO_ERROR == status ) {
         status = ioByteBuffer_putString( target, key );
         if( UTIL_STATUS_NO_ERROR == status ) {
            if( type == dcrudTYPE_SHAREABLE ) {
               dcrudShareableImpl * item = (dcrudShareableImpl *)value;
               status = dcrudClassID_serialize( item->classID, target );
               if( UTIL_STATUS_NO_ERROR == status ) {
                  status = item->factory->serialize((dcrudShareable)item, target );
               }
            }
            else {
               status = dcrudClassID_serializeType( type, target );
               if( UTIL_STATUS_NO_ERROR == status ) {
                  switch( type ) {
                  case dcrudTYPE_NULL            : /* No Data. */ break;
                  case dcrudTYPE_BOOLEAN         :
                  case dcrudTYPE_BYTE            :
                  case dcrudTYPE_CHAR            : status = ioByteBuffer_putByte  ( target, value->Byte    ); break;
                  case dcrudTYPE_SHORT           : status = ioByteBuffer_putShort ( target, value->Short   ); break;
                  case dcrudTYPE_UNSIGNED_SHORT  : status = ioByteBuffer_putUShort( target, value->Ushort  ); break;
                  case dcrudTYPE_INTEGER         : status = ioByteBuffer_putInt   ( target, value->Int     ); break;
                  case dcrudTYPE_UNSIGNED_INTEGER: status = ioByteBuffer_putUInt  ( target, value->Uint    ); break;
                  case dcrudTYPE_LONG            : status = ioByteBuffer_putLong  ( target, value->Long    ); break;
                  case dcrudTYPE_UNSIGNED_LONG   : status = ioByteBuffer_putULong ( target, value->Ulong   ); break;
                  case dcrudTYPE_FLOAT           : status = ioByteBuffer_putFloat ( target, value->Float   ); break;
                  case dcrudTYPE_DOUBLE          : status = ioByteBuffer_putDouble( target, value->Double  ); break;
                  case dcrudTYPE_STRING          : status = ioByteBuffer_putString( target, value->String  ); break;
                  case dcrudTYPE_CLASS_ID        : status = dcrudClassID_serialize( value->ClassID, target ); break;
                  case dcrudTYPE_GUID            : status = dcrudGUID_serialize   ( value->GUID   , target ); break;
                  case dcrudTYPE_SHAREABLE       : /* Already handled before this switch. */
                  case dcrudLAST_TYPE            : status = UTIL_STATUS_ILLEGAL_STATE; break;
                  }
               }
            }
         }
      }
   }
   return status;
}

utilStatus dcrudArguments_serialize( dcrudArguments self, ioByteBuffer target ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( NULL == target ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudArgumentsImpl * This = dcrudArguments_safeCast( self, &status );
      if( status == UTIL_STATUS_NO_ERROR ) {
         Context ctxt;
         CHK(__FILE__,__LINE__,ioByteBuffer_putByte( target, This->mode ))
         CHK(__FILE__,__LINE__,ioByteBuffer_putByte( target, This->queue ))
         ctxt.source = This;
         ctxt.buffer = target;
         ctxt.file   = NULL;
         status = collMap_foreach( This->args, serializePair, &ctxt );
      }
   }
   return status;
}

static utilStatus printPair( collForeach * context ) {
   Context *                   ctxt   = (Context *                   )context->user;
   const char *                key    = (const char *                )context->key;
   const dcrudArgumentsValue * value  = (const dcrudArgumentsValue * )context->value;
   dcrudArgumentsImpl *        source = ctxt->source;
   FILE *                      target = ctxt->file;
   dcrudType                   type;
   utilStatus                  status = UTIL_STATUS_NO_ERROR;

   fprintf( target, "%20s => ", key );
   CHK(__FILE__,__LINE__,dcrudArguments_getType((dcrudArguments)source, key, &type ))
   if( type  == dcrudTYPE_NULL ) {
      fprintf( target, "null          : null\n" );
   }
   else if( value ) {
      if( type == dcrudTYPE_SHAREABLE ) {
         dcrudShareableImpl * shareable = (dcrudShareableImpl *)value->Shareable;
         if( shareable ) {
            char cs[20];
            char id[20];

            CHK(__FILE__,__LINE__,dcrudClassID_toString( shareable->classID, cs, sizeof( cs )))
            CHK(__FILE__,__LINE__,dcrudGUID_toString   ( shareable->id     , id, sizeof( id )))
            fprintf( target, "%s, %s\n", cs, id );
         }
         else {
            fprintf( target, "null          : null\n" );
         }
      }
      else {
         char cs[20];
         char id[20];
         switch( type ) {
         case dcrudTYPE_NULL            : fprintf( target, "null\n" ); break;
         case dcrudTYPE_BOOLEAN         : fprintf( target, "boolean       : %s\n"       , value->Boolean ? "true" : "false" ); break;
         case dcrudTYPE_BYTE            : fprintf( target, "byte          : %d\n"       , value->Byte   ); break;
         case dcrudTYPE_CHAR            : fprintf( target, "char          : %c\n"       , value->Char   ); break;
         case dcrudTYPE_SHORT           : fprintf( target, "short         : %d\n"       , value->Short  ); break;
         case dcrudTYPE_UNSIGNED_SHORT  : fprintf( target, "unsigned short: %u\n"       , value->Ushort ); break;
         case dcrudTYPE_INTEGER         : fprintf( target, "int           : %d\n"       , value->Int    ); break;
         case dcrudTYPE_UNSIGNED_INTEGER: fprintf( target, "unsigned int  : %u\n"       , value->Uint   ); break;
         case dcrudTYPE_LONG            : fprintf( target, "long          : %"PRId64"\n", value->Long   ); break;
         case dcrudTYPE_UNSIGNED_LONG   : fprintf( target, "unsigned long : %"PRIu64"\n", value->Ulong  ); break;
         case dcrudTYPE_FLOAT           : fprintf( target, "float         : %f\n"       , value->Float  ); break;
         case dcrudTYPE_DOUBLE          : fprintf( target, "double        : %f\n"       , value->Double ); break;
         case dcrudTYPE_STRING          : fprintf( target, "string        : %s\n"       , value->String ); break;
         case dcrudTYPE_CLASS_ID        :
            CHK(__FILE__,__LINE__,dcrudClassID_toString( value->ClassID, cs, sizeof( cs )))
            fprintf( target, "ClassID       : %s\n", cs );
            break;
         case dcrudTYPE_GUID            :
            CHK(__FILE__,__LINE__,dcrudGUID_toString( value->GUID, id, sizeof( id )))
            fprintf( target, "GUID          : %s\n", id );
            break;
         default:
            fprintf( target, "%d: %p !!!\n", type, (const void *)value );
            break;
         }
      }
   }
   else {
      fprintf( target, "???       : null\n" );
   }
   return status;
}

utilStatus dcrudArguments_dump( dcrudArguments self, FILE * target ) {
   utilStatus           status = UTIL_STATUS_NO_ERROR;
   dcrudArgumentsImpl * This   = dcrudArguments_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      Context      ctxt;
      unsigned int count = 0;
      status = dcrudArguments_getCount( self, &count );
      if( status == UTIL_STATUS_NO_ERROR ) {
         if( NULL == target ) {
            target = stderr;
         }
         ctxt.source = This;
         ctxt.buffer = 0;
         ctxt.file   = target;
         fprintf( target, "mode : %d\n", This->mode );
         fprintf( target, "queue: %d\n", This->queue );
         fprintf( target, "#%d\n", count );
         status = collMap_foreach( This->args, printPair, &ctxt );
      }
   }
   return status;
}

utilStatus dcrudArguments_putNull( dcrudArguments self, const char * key ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( NULL == key ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudArgumentsImpl * This = dcrudArguments_safeCast( self, &status );
      if( status == UTIL_STATUS_NO_ERROR ) {
         char *      allocateKey = NULL;
         collMapPair prev;
         CHK(__FILE__,__LINE__,utilString_dup( &allocateKey, key ))
         CHK(__FILE__,__LINE__,collMap_put( This->args , allocateKey, NULL    , &prev ))
         CHK(__FILE__,__LINE__,collMap_put( This->types, allocateKey, &typNULL, NULL ))
         if( prev.key ) {
            collForeach context;
            context.user  = This;
            context.key   = prev.key;
            context.value = prev.value;
            releaseValue( &context );
         }
      }
   }
   return status;
}

#ifdef STATIC_ALLOCATION
#define DCRUD_ARGS_DECLARE_PUT( N, T, D )\
utilStatus dcrudArguments_put##N( dcrudArguments self, const char * key, T t ) {\
   utilStatus status = UTIL_STATUS_NO_ERROR;\
   if( NULL == key ) {\
      status = UTIL_STATUS_NULL_ARGUMENT;\
   }\
   else {\
      dcrudArgumentsImpl * This = dcrudArguments_safeCast( self, &status );\
      if( UTIL_STATUS_NO_ERROR == status ) {\
         if( dcrudArgumentsValuePoolInit ) {\
            status = utilPool_new(\
               &dcrudArgumentsValuePool,\
               "dcrudArgumentsValue",\
               dcrudArgumentsValuePoolData,\
               sizeof( dcrudArgumentsValue ),\
               dcrudArgumentsValuePoolEntries,\
               dcrudArgumentsValuePoolSize );\
            if( UTIL_STATUS_NO_ERROR == status ) {\
               dcrudArgumentsValuePoolInit = false;\
            }\
         }\
         if( UTIL_STATUS_NO_ERROR == status ) {\
            collMapPair           prev;\
            dcrudArgumentsValue * value       = NULL;\
            char *                allocateKey = NULL;\
            CHK(__FILE__,__LINE__,utilPool_reserve( &dcrudArgumentsValuePool, &value ))\
            CHK(__FILE__,__LINE__,utilString_dup( &allocateKey, key ))\
            memset( value, 0, sizeof( dcrudArgumentsValue ));\
            value->N = t;\
            CHK(__FILE__,__LINE__,collMap_put( This->args, allocateKey, value, &prev ))\
            CHK(__FILE__,__LINE__,collMap_put( This->types, allocateKey, &typ##D, NULL ))\
            if( prev.key ) {\
               collForeach context;\
               context.user  = This;\
               context.key   = prev.key;\
               context.value = prev.value;\
               releaseValue( &context );\
            }\
         }\
      }\
   }\
   return status;\
}
#else
#define DCRUD_ARGS_DECLARE_PUT( N, T, D )\
utilStatus dcrudArguments_put##N( dcrudArguments self, const char * key, T t ) {\
   utilStatus status = UTIL_STATUS_NO_ERROR;\
   if( NULL == key ) {\
      status = UTIL_STATUS_NULL_ARGUMENT;\
   }\
   else {\
      dcrudArgumentsImpl * This = dcrudArguments_safeCast( self, &status );\
      if( UTIL_STATUS_NO_ERROR == status ) {\
         if( UTIL_STATUS_NO_ERROR == status ) {\
            collMapPair           prev;\
            dcrudArgumentsValue * argsValue   = NULL;\
            char *                allocateKey = NULL;\
            argsValue = (dcrudArgumentsValue *)malloc( sizeof( dcrudArgumentsValue ));\
            if( NULL == argsValue ) {\
               return UTIL_STATUS_TOO_MANY;\
            }\
            memset( argsValue, 0, sizeof( dcrudArgumentsValue ));\
            CHK(__FILE__,__LINE__,utilString_dup( &allocateKey, key ))\
            argsValue->N = t;\
            CHK(__FILE__,__LINE__,collMap_put( This->args, allocateKey, argsValue, &prev ))\
            CHK(__FILE__,__LINE__,collMap_put( This->types, allocateKey, &typ##D, NULL ))\
            if( prev.key ) {\
               collForeach context;\
               context.user  = This;\
               context.key   = prev.key;\
               context.value = prev.value;\
               releaseValue( &context );\
            }\
         }\
      }\
   }\
   return status;\
}
#endif

utilStatus dcrudArguments_putString( dcrudArguments self, const char * key, const char * value ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if(( NULL == key )||( NULL == value )) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudArgumentsImpl * This = dcrudArguments_safeCast( self, &status );
      if( UTIL_STATUS_NO_ERROR == status ) {
#ifdef STATIC_ALLOCATION
         if( dcrudArgumentsValuePoolInit ) {
            status = utilPool_new(
               &dcrudArgumentsValuePool,
               "dcrudArgumentsValue",
               dcrudArgumentsValuePoolData,
               sizeof( dcrudArgumentsValue ),
               dcrudArgumentsValuePoolEntries,
               dcrudArgumentsValuePoolSize );
            if( UTIL_STATUS_NO_ERROR == status ) {
               dcrudArgumentsValuePoolInit = false;
            }
         }
#endif
         if( UTIL_STATUS_NO_ERROR == status ) {
            collMapPair           prev;
            char *                allocateKey = NULL;
            dcrudArgumentsValue * argsValue   = NULL;

#ifdef STATIC_ALLOCATION
            CHK(__FILE__,__LINE__,utilPool_reserve( &dcrudArgumentsValuePool, &argsValue ))
#else
            argsValue = (dcrudArgumentsValue *)malloc( sizeof( dcrudArgumentsValue ));
            if( NULL == argsValue ) {
               return UTIL_STATUS_TOO_MANY;
            }
#endif
            memset( argsValue, 0, sizeof( dcrudArgumentsValue ));
            CHK(__FILE__,__LINE__,utilString_dup( &allocateKey      , key   ))
            CHK(__FILE__,__LINE__,utilString_dup( &argsValue->String, value ))
            CHK(__FILE__,__LINE__,collMap_put( This->args , allocateKey, argsValue , &prev ))
            CHK(__FILE__,__LINE__,collMap_put( This->types, allocateKey, &typSTRING, NULL  ))
            if( prev.key ) {
               collForeach context;
               context.user  = This;
               context.key   = prev.key;
               context.value = prev.value;
               releaseValue( &context );
            }
         }
      }
   }
   return status;
}

DCRUD_ARGS_DECLARE_PUT( Boolean  , bool          , BOOLEAN   )
DCRUD_ARGS_DECLARE_PUT( Byte     , byte          , BYTE      )
DCRUD_ARGS_DECLARE_PUT( Char     , char          , CHAR      )
DCRUD_ARGS_DECLARE_PUT( Short    , short         , SHORT     )
DCRUD_ARGS_DECLARE_PUT( Ushort   , unsigned short, USHORT    )
DCRUD_ARGS_DECLARE_PUT( Int      , int           , INTEGER   )
DCRUD_ARGS_DECLARE_PUT( Uint     , unsigned int  , UINTEGER  )
DCRUD_ARGS_DECLARE_PUT( Long     , int64_t       , LONG      )
DCRUD_ARGS_DECLARE_PUT( Ulong    , uint64_t      , ULONG     )
DCRUD_ARGS_DECLARE_PUT( Float    , float         , FLOAT     )
DCRUD_ARGS_DECLARE_PUT( Double   , double        , DOUBLE    )
DCRUD_ARGS_DECLARE_PUT( ClassID  , dcrudClassID  , CLASS_ID  )
DCRUD_ARGS_DECLARE_PUT( GUID     , dcrudGUID     , GUID      )
DCRUD_ARGS_DECLARE_PUT( Shareable, dcrudShareable, SHAREABLE )

utilStatus dcrudArguments_isNull( dcrudArguments self, const char * key, bool * result ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( result == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudArgumentsImpl * This = dcrudArguments_safeCast( self, &status );
      if( status == UTIL_STATUS_NO_ERROR ) {
         collMapValue value = NULL;
         status = collMap_get( This->args, (collMapKey)key, &value );
         if( status == UTIL_STATUS_NO_ERROR ) {
            dcrudArgumentsValue * param = (dcrudArgumentsValue *)value;
            *result = ( value == NULL || param->Shareable == NULL );
         }
      }
   }
   return status;
}

utilStatus dcrudArguments_getType( dcrudArguments self, const char * key, dcrudType * result ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( result == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudArgumentsImpl * This = dcrudArguments_safeCast( self, &status );
      if( status == UTIL_STATUS_NO_ERROR ) {
         collMapValue value = NULL;
         CHK(__FILE__,__LINE__,collMap_get( This->types, (collMapKey)key, &value ))
         *result = *(dcrudType *)value;
      }
   }
   return status;
}

#define DCRUD_ARGS_DECLARE_GET( N, T )\
utilStatus dcrudArguments_get##N( dcrudArguments self, const char * key, T * result ) {\
   utilStatus status = UTIL_STATUS_NO_ERROR;\
   if( result == NULL ) {\
      status = UTIL_STATUS_NULL_ARGUMENT;\
   }\
   else {\
      dcrudArgumentsImpl * This = dcrudArguments_safeCast( self, &status );\
      if( status == UTIL_STATUS_NO_ERROR ) {\
         collMapValue value = NULL;\
         status = collMap_get( This->args, (collMapKey)key, &value );\
         if( status == UTIL_STATUS_NO_ERROR ) {\
            if( value != NULL ) {\
               dcrudArgumentsValue * param = (dcrudArgumentsValue *)value;\
               *result = param->N;\
            }\
            else {\
               status = UTIL_STATUS_NOT_FOUND;\
            }\
         }\
      }\
   }\
   return status;\
}

DCRUD_ARGS_DECLARE_GET( Boolean  , bool           )
DCRUD_ARGS_DECLARE_GET( Byte     , byte           )
DCRUD_ARGS_DECLARE_GET( Char     , char           )
DCRUD_ARGS_DECLARE_GET( Short    , short          )
DCRUD_ARGS_DECLARE_GET( Ushort   , unsigned short )
DCRUD_ARGS_DECLARE_GET( Int      , int            )
DCRUD_ARGS_DECLARE_GET( Uint     , unsigned int   )
DCRUD_ARGS_DECLARE_GET( Long     , int64_t        )
DCRUD_ARGS_DECLARE_GET( Ulong    , uint64_t       )
DCRUD_ARGS_DECLARE_GET( Float    , float          )
DCRUD_ARGS_DECLARE_GET( Double   , double         )
DCRUD_ARGS_DECLARE_GET( ClassID  , dcrudClassID   )
DCRUD_ARGS_DECLARE_GET( GUID     , dcrudGUID      )
DCRUD_ARGS_DECLARE_GET( Shareable, dcrudShareable )

utilStatus dcrudArguments_getString(
   dcrudArguments self,
   const char *   key,
   char *         target,
   size_t         targetSize )
{
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( target == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudArgumentsImpl * This = dcrudArguments_safeCast( self, &status );
      if( status == UTIL_STATUS_NO_ERROR ) {
         collMapValue value = NULL;
         status = collMap_get( This->args, (collMapKey)key, &value );
         if( status == UTIL_STATUS_NO_ERROR ) {
            if( value != NULL ) {
               dcrudArgumentsValue * param = (dcrudArgumentsValue *)value;
               strncpy( target, param->String, targetSize );
            }
            else {
               status = UTIL_STATUS_NOT_FOUND;
            }
         }
      }
   }
   return status;
}
