#include <dcrud/Arguments.h>
#include <coll/Map.h>
#include <coll/Comparator.h>
#include "Shareable_private.h"

typedef struct Arguments_t {

   collMap args;
   collMap types;

} Arguments;

static dcrudType typNULL        = dcrudTYPE_NULL;
static dcrudType typBYTE        = dcrudTYPE_BYTE;
static dcrudType typBOOLEAN     = dcrudTYPE_BOOLEAN;
static dcrudType typSHORT       = dcrudTYPE_SHORT;
static dcrudType typINTEGER     = dcrudTYPE_INTEGER;
static dcrudType typLONG        = dcrudTYPE_LONG;
static dcrudType typFLOAT       = dcrudTYPE_FLOAT;
static dcrudType typDOUBLE      = dcrudTYPE_DOUBLE;
static dcrudType typSTRING      = dcrudTYPE_STRING;
static dcrudType typCLASS_ID    = dcrudTYPE_CLASS_ID;
static dcrudType typGUID        = dcrudTYPE_GUID;
static dcrudType typCALL_MODE   = dcrudTYPE_CALL_MODE;
static dcrudType typQUEUE_INDEX = dcrudTYPE_QUEUE_INDEX;
static dcrudType typSHAREABLE   = dcrudTYPE_SHAREABLE;

static void freePair( collMapPair * pair, collMap types ) {
   if( pair->key ) {
      dcrudType * type = collMap_get( types, pair->key );
      if( type && pair->value ) {
         switch( *type ) {
         case dcrudTYPE_NULL       :
         case dcrudTYPE_BYTE       :
         case dcrudTYPE_BOOLEAN    :
         case dcrudTYPE_SHORT      :
         case dcrudTYPE_INTEGER    :
         case dcrudTYPE_LONG       :
         case dcrudTYPE_FLOAT      :
         case dcrudTYPE_DOUBLE     :
         case dcrudTYPE_STRING     :
         case dcrudTYPE_CALL_MODE  :
         case dcrudTYPE_QUEUE_INDEX:
            free( pair->value );
            break;
         case dcrudTYPE_CLASS_ID   :
         case dcrudTYPE_GUID       :
         case dcrudTYPE_SHAREABLE  :
         case dcrudLAST_TYPE       :
            /* Do nothing */
            break;
         }
         free( pair->key );
      }
   }
}

static bool foreach_freePair( collForeach * context ) {
   collMap       types = (collMap      )context->user;
   collMapPair * pair  = (collMapPair *)context->item;
   freePair( pair, types );
   return true;
}

static void * copy( const void * data, size_t size ) {
   void * m = malloc( size );
   memcpy( m, data, size );
   return m;
}

dcrudArguments dcrudArguments_new( void ) {
   Arguments * This = (Arguments *)malloc( sizeof( Arguments ));
   memset( This, 0, sizeof( Arguments ));
   This->args  = collMap_new((collComparator)collStringComparator );
   This->types = collMap_new((collComparator)collStringComparator );
   return (dcrudArguments)This;
}

void dcrudArguments_delete( dcrudArguments * self ) {
   Arguments * This = *(Arguments **)self;
   collMap_foreach( This->args, foreach_freePair, This->types );
}

void dcrudArguments_clear( dcrudArguments self ) {
   Arguments * This = (Arguments *)self;
   collMap_clear( This->args );
}

bool dcrudArguments_isEmpty( dcrudArguments self ) {
   Arguments * This = (Arguments *)self;
   return collMap_size( This->args ) == 0;
}

unsigned int dcrudArguments_getCount( dcrudArguments self ) {
   Arguments * This = (Arguments *)self;
   return This ? collMap_size( This->args ) : 0;
}

void dcrudArguments_setMode( dcrudArguments self, byte value ) {
   Arguments * This = (Arguments *)self;
   char *      key  = strdup( "@mode" );
   collMapPair prev;

   collMap_put( This->args , key, copy( &value, sizeof( value )), &prev );
   collMap_put( This->types, key, &typCALL_MODE, NULL );
   freePair( &prev, This->types );
}

void dcrudArguments_setQueue( dcrudArguments self, byte value ) {
   Arguments * This = (Arguments *)self;
   char *      key  = strdup( "@queue" );
   collMapPair prev;

   collMap_put( This->args , key, copy( &value, sizeof( value )), &prev );
   collMap_put( This->types, key, &typQUEUE_INDEX, NULL );
   freePair( &prev, This->types );
}

void dcrudArguments_putNull( dcrudArguments self, const char * key ) {
   Arguments * This = (Arguments *)self;
   char *      k    = strdup( key );
   collMapPair prev;

   collMap_put( This->args , k, 0, &prev );
   collMap_put( This->types, k, &typNULL, NULL );
   freePair( &prev, This->types );
}

#define DCRUD_ARGS_DECLARE_PUT( N, T, D )\
void dcrudArguments_put##N( dcrudArguments self, const char * key, T value ) {\
   Arguments * This = (Arguments *)self;\
   char *      k    = strdup( key );\
   collMapPair prev;\
\
   collMap_put( This->args , k, copy( &value, sizeof( value )), &prev );\
   collMap_put( This->types, k, &typ##D, NULL );\
   freePair( &prev, This->types );\
}

DCRUD_ARGS_DECLARE_PUT( Byte   , byte          , BYTE    )
DCRUD_ARGS_DECLARE_PUT( Boolean, bool          , BOOLEAN )
DCRUD_ARGS_DECLARE_PUT( Short  , unsigned short, SHORT   )
DCRUD_ARGS_DECLARE_PUT( Int    , unsigned int  , INTEGER )
DCRUD_ARGS_DECLARE_PUT( Long   , uint64_t      , LONG    )
DCRUD_ARGS_DECLARE_PUT( Float  , float         , FLOAT   )
DCRUD_ARGS_DECLARE_PUT( Double , double        , DOUBLE  )

#undef DCRUD_ARGS_DECLARE_PUT

void dcrudArguments_putString( dcrudArguments self, const char * key, const char * value ) {
   Arguments * This = (Arguments *)self;
   char *      k    = strdup( key );
   collMapPair prev;

   collMap_put( This->args , k, strdup( value ), &prev );
   collMap_put( This->types, k, &typSTRING, NULL );
   freePair( &prev, This->types );
}

void dcrudArguments_putClassID( dcrudArguments self, const char * key, dcrudClassID value ) {
   Arguments * This = (Arguments *)self;
   char *      k    = strdup( key );
   collMapPair prev;

   collMap_put( This->args , k, value, &prev );
   collMap_put( This->types, k, &typCLASS_ID, NULL );
   freePair( &prev, This->types );
}

void dcrudArguments_putGUID( dcrudArguments self, const char * key, dcrudGUID value ) {
   Arguments * This = (Arguments *)self;
   char *      k    = strdup( key );
   collMapPair prev;

   collMap_put( This->args , k, value, &prev );
   collMap_put( This->types, k, &typGUID, NULL );
   freePair( &prev, This->types );
}

void dcrudArguments_putShareable( dcrudArguments self, const char * key, dcrudShareable value ) {
   Arguments * This = (Arguments *)self;
   char *      k    = strdup( key );
   collMapPair prev;

   collMap_put( This->args , k, value, &prev );
   collMap_put( This->types, k, &typSHAREABLE, NULL );
   freePair( &prev, This->types );
}

bool dcrudArguments_isNull( dcrudArguments self, const char * key ) {
   Arguments *  This  = (Arguments *)self;
   const void * value = collMap_get( This->args, (collMapKey)key );
   return !value;
}

#define DCRUD_ARGS_DECLARE_GET( N, T )\
bool dcrudArguments_get##N( dcrudArguments self, const char * key, T * value ) {\
   Arguments *  This = (Arguments *)self;\
   const void * data = collMap_get( This->args, (collMapKey)key );\
   if( data ) {\
      *value = *(const T *)data;\
   }\
   return data != 0;\
}

DCRUD_ARGS_DECLARE_GET( Byte   , byte           )
DCRUD_ARGS_DECLARE_GET( Boolean, bool           )
DCRUD_ARGS_DECLARE_GET( Short  , unsigned short )
DCRUD_ARGS_DECLARE_GET( Int    , unsigned int   )
DCRUD_ARGS_DECLARE_GET( Long   , uint64_t       )
DCRUD_ARGS_DECLARE_GET( Float  , float          )
DCRUD_ARGS_DECLARE_GET( Double , double         )

#undef DCRUD_ARGS_DECLARE_GET

bool dcrudArguments_getString( dcrudArguments self, const char * key, char * buffer, unsigned int capacity ) {
   Arguments *  This = (Arguments *)self;
   const void * data = collMap_get( This->args, (collMapKey)key );
   if( data ) {
      strncpy( buffer, (const char *)data, capacity );
   }
   return data != 0;
}

bool dcrudArguments_getClassID( dcrudArguments self, const char * key, dcrudClassID * value ) {
   Arguments *  This = (Arguments *)self;
   const void * data = collMap_get( This->args, (collMapKey)key );
   if( data ) {
      *value = (const dcrudClassID)data;
   }
   return data != 0;
}

bool dcrudArguments_getGUID( dcrudArguments self, const char * key, dcrudGUID * value ) {
   Arguments *  This = (Arguments *)self;
   const void * data = collMap_get( This->args, (collMapKey)key );
   if( data ) {
      *value = (const dcrudGUID)data;
   }
   return data != 0;
}

bool dcrudArguments_getShareable( dcrudArguments self, const char * key, dcrudShareable * value ) {
   Arguments *  This = (Arguments *)self;
   const void * data = collMap_get( This->args, (collMapKey)key );
   if( data ) {
      *value = (const dcrudShareable)data;
   }
   return data != 0;
}

dcrudType dcrudArguments_getType( dcrudArguments self, const char * key ) {
   Arguments *  This = (Arguments *)self;
   collMapValue type = collMap_get( This->types, (collMapKey)key );
   if( type ) {
      return *(const dcrudType *)type;
   }
   return dcrudLAST_TYPE;
}

typedef struct Context_t {

   Arguments *  source;
   ioByteBuffer target;

} Context;

static bool serializePair( collForeach * context ) {
   Context *         ctxt   = (Context *  )context->user;
   Arguments *       source = ctxt->source;
   ioByteBuffer      target = ctxt->target;
   collMapPair *     pair   = (collMapPair *)context->item;
   const char *      name   = (const char * )pair->key;
   const void *      value  = (const void * )pair->value;
   const dcrudType * type   = (const dcrudType *)collMap_get( source->types, (collMapKey)name );

   if( ! type ) {
      fprintf( stderr, "%s:%d: No type for %s\n", __FILE__, __LINE__, name );
      return false;
   }
   if( value ) {
      ioByteBuffer_putString( target, name );
      if( *type == dcrudTYPE_SHAREABLE ) {
         dcrudShareableImpl * item = (dcrudShareableImpl *)value;
         dcrudClassID_serialize( item->classID, target );
         item->serialize((dcrudShareable)item, target );
      }
      else {
         dcrudClassID_serializeType( *type, target );
         switch( *type ) {
         case dcrudTYPE_NULL       : /* No Data. */ break;
         case dcrudTYPE_BYTE       :
         case dcrudTYPE_BOOLEAN    : ioByteBuffer_putByte  ( target, *(byte *          )value ); break;
         case dcrudTYPE_SHORT      : ioByteBuffer_putShort ( target, *(unsigned short *)value ); break;
         case dcrudTYPE_INTEGER    : ioByteBuffer_putInt   ( target, *(unsigned int *  )value ); break;
         case dcrudTYPE_LONG       : ioByteBuffer_putLong  ( target, *(uint64_t *      )value ); break;
         case dcrudTYPE_FLOAT      : ioByteBuffer_putFloat ( target, *(float *         )value ); break;
         case dcrudTYPE_DOUBLE     : ioByteBuffer_putDouble( target, *(double *        )value ); break;
         case dcrudTYPE_STRING     : ioByteBuffer_putString( target, (const char *     )value ); break;
         case dcrudTYPE_CLASS_ID   : dcrudClassID_serialize((const dcrudClassID)value, target ); break;
         case dcrudTYPE_GUID       : dcrudGUID_serialize   ((const dcrudGUID   )value, target ); break;
         case dcrudTYPE_CALL_MODE  : ioByteBuffer_putByte  ( target, *(byte *          )value ); break;
         case dcrudTYPE_QUEUE_INDEX: ioByteBuffer_putByte  ( target, *(byte *          )value ); break;
         case dcrudTYPE_SHAREABLE  : /* Already handled before this switch. */
         case dcrudLAST_TYPE       : break;
         }
      }
   }
   return true;
}

bool dcrudArguments_serialize( dcrudArguments self, ioByteBuffer target ) {
   Arguments * This = (Arguments *)self;
   if( This ) {
      Context   ctxt;
      ctxt.source = This;
      ctxt.target = target;
      collMap_foreach( This->args, serializePair, &ctxt );
      return true;
   }
   return false;
}
