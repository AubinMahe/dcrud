#include <dcrud/Arguments.h>
#include <dcrud/IRequired.h>
#include <dcrud/GUID_private.h>
#include <string.h>

static const char * OK   = "OK";
static const char * FAIL = "FAIL";

static bool           BOOLEAN_VALUE = true;
static byte           BYTE_VALUE    = 1;
static short          SHORT_VALUE   = 2;
static unsigned short USHORT_VALUE  = 3U;
static int            INT_VALUE     = 4;
static unsigned int   UINT_VALUE    = 5U;
static int64_t        LONG_VALUE    = 6;
static uint64_t       ULONG_VALUE   = 7U;
static float          FLOAT_VALUE   = 8.9;
static double         DOUBLE_VALUE  = 10.11;
static char *         STRING_VALUE  = "Hello, World!";
static dcrudClassID   CLASS_ID_VALUE;
static dcrudGUID      GUID_VALUE;
static dcrudShareable SHAREABLE_VALUE;

#define PRT(F,O,E) printf( "%-4s: " # O " == %" F "\n", ((E) == (O)) ? OK : FAIL, E );

utilStatus dcrudArgumentsTest( void ) {
   dcrudArguments  args = NULL;
   bool            isEmpty = false;
   unsigned int    count   = 12;
   bool            isNull  = true;
   dcrudCallMode   mode;
   dcrudQueueIndex queue;
   byte            byteValue;
   bool            boolValue;
   short           shortValue;
   unsigned short  ushortValue;
   int             intValue;
   unsigned int    uintValue;
   int64_t         int64Value;
   uint64_t        uint64Value;
   float           floatValue;
   double          doubleValue;
   char            buffer[1024];
   dcrudClassID    dcrudClassIDValue;
   dcrudGUID       dcrudGUIDValue;
   dcrudShareable  dcrudShareableValue;
   dcrudType       dcrudTypeValue = dcrudLAST_TYPE;

   CHK(__FILE__,__LINE__,dcrudClassID_new( &CLASS_ID_VALUE, 1, 2, 3, 4 ))
   CHK(__FILE__,__LINE__,dcrudGUID_new( &GUID_VALUE ))
   CHK(__FILE__,__LINE__,dcrudGUID_init( GUID_VALUE, 42, 84 ))
   SHAREABLE_VALUE = NULL;

   CHK(__FILE__,__LINE__,dcrudArguments_new( &args ))

   CHK(__FILE__,__LINE__,dcrudArguments_isEmpty( args, &isEmpty ))
   printf( "%-4s: isEmpty\n", isEmpty ? OK : FAIL );
   CHK(__FILE__,__LINE__,dcrudArguments_getCount( args, &count ))
   printf( "%-4s: getCount\n", ( count == 0 ) ? OK : FAIL );
   CHK(__FILE__,__LINE__,dcrudArguments_getMode ( args, &mode ))
   printf( "%-4s: default mode is DCRUD_ASYNCHRONOUS_DEFERRED\n", ( mode == DCRUD_ASYNCHRONOUS_DEFERRED ) ? OK : FAIL );
   CHK(__FILE__,__LINE__,dcrudArguments_getQueue( args, &queue ))
   printf( "%-4s: default queue is DCRUD_DEFAULT_QUEUE\n", ( queue == DCRUD_DEFAULT_QUEUE ) ? OK : FAIL );
   CHK(__FILE__,__LINE__,dcrudArguments_setMode     ( args, DCRUD_ASYNCHRONOUS_IMMEDIATE ))
   CHK(__FILE__,__LINE__,dcrudArguments_setQueue    ( args, DCRUD_URGENT_QUEUE ))
   CHK(__FILE__,__LINE__,dcrudArguments_putNull     ( args, "null-key" ))
   CHK(__FILE__,__LINE__,dcrudArguments_putBoolean  ( args, "boolean-key"  , BOOLEAN_VALUE   ))
   CHK(__FILE__,__LINE__,dcrudArguments_putByte     ( args, "byte-key"     , BYTE_VALUE      ))
   CHK(__FILE__,__LINE__,dcrudArguments_putShort    ( args, "short-key"    , SHORT_VALUE     ))
   CHK(__FILE__,__LINE__,dcrudArguments_putUshort   ( args, "ushort-key"   , USHORT_VALUE    ))
   CHK(__FILE__,__LINE__,dcrudArguments_putInt      ( args, "int-key"      , INT_VALUE       ))
   CHK(__FILE__,__LINE__,dcrudArguments_putUint     ( args, "uint-key"     , UINT_VALUE      ))
   CHK(__FILE__,__LINE__,dcrudArguments_putLong     ( args, "long-key"     , LONG_VALUE      ))
   CHK(__FILE__,__LINE__,dcrudArguments_putUlong    ( args, "ulong-key"    , ULONG_VALUE     ))
   CHK(__FILE__,__LINE__,dcrudArguments_putFloat    ( args, "float-key"    , FLOAT_VALUE     ))
   CHK(__FILE__,__LINE__,dcrudArguments_putDouble   ( args, "double-key"   , DOUBLE_VALUE    ))
   CHK(__FILE__,__LINE__,dcrudArguments_putString   ( args, "string-key"   , STRING_VALUE    ))
   CHK(__FILE__,__LINE__,dcrudArguments_putClassID  ( args, "class-id-key" , CLASS_ID_VALUE  ))
   CHK(__FILE__,__LINE__,dcrudArguments_putGUID     ( args, "guid-key"     , GUID_VALUE      ))
   CHK(__FILE__,__LINE__,dcrudArguments_putShareable( args, "shareable-key", SHAREABLE_VALUE ))
   CHK(__FILE__,__LINE__,dcrudArguments_dump        ( args, stdout ))
   CHK(__FILE__,__LINE__,dcrudArguments_isEmpty     ( args, &isEmpty ))
   printf( "%-4s: isEmpty\n", isEmpty ? FAIL : OK );
   CHK(__FILE__,__LINE__,dcrudArguments_getCount    ( args, &count ))
   printf( "%-4s: getCount\n", count == 15 ? OK : FAIL );
   CHK(__FILE__,__LINE__,dcrudArguments_getMode ( args, &mode ))
   printf( "%-4s: default mode is DCRUD_ASYNCHRONOUS_IMMEDIATE\n", ( mode == DCRUD_ASYNCHRONOUS_IMMEDIATE ) ? OK : FAIL );
   CHK(__FILE__,__LINE__,dcrudArguments_getQueue( args, &queue ))
   printf( "%-4s: default queue is DCRUD_URGENT_QUEUE\n", ( queue == DCRUD_URGENT_QUEUE ) ? OK : FAIL );
   CHK(__FILE__,__LINE__,dcrudArguments_isNull      ( args, "short-key"    , &isNull              ))
   printf( "%-4s: isNull\n", isNull ? FAIL : OK );
   CHK(__FILE__,__LINE__,dcrudArguments_isNull      ( args, "shareable-key", &isNull              ))
   printf( "%-4s: isNull\n", isNull ? OK : FAIL );
   CHK(__FILE__,__LINE__,dcrudArguments_getBoolean  ( args, "boolean-key"  , &boolValue           ))
   printf( "%-4s: boolValue == true\n"  , boolValue ? OK : FAIL );
   CHK(__FILE__,__LINE__,dcrudArguments_getByte     ( args, "byte-key"     , &byteValue           ))
   PRT( "u", byteValue, BYTE_VALUE )
   printf( "%-4s: byteValue == %d\n"    , ( BYTE_VALUE   == byteValue   ) ? OK : FAIL, BYTE_VALUE );
   CHK(__FILE__,__LINE__,dcrudArguments_getShort    ( args, "short-key"    , &shortValue          ))
   PRT( "d", shortValue         , SHORT_VALUE );
   CHK(__FILE__,__LINE__,dcrudArguments_getUshort   ( args, "ushort-key"   , &ushortValue         ))
   PRT( "u", ushortValue        , USHORT_VALUE );
   CHK(__FILE__,__LINE__,dcrudArguments_getInt      ( args, "int-key"      , &intValue            ))
   PRT( "d", intValue           , INT_VALUE );
   CHK(__FILE__,__LINE__,dcrudArguments_getUint     ( args, "uint-key"     , &uintValue           ))
   PRT( "u", uintValue          , UINT_VALUE );
   CHK(__FILE__,__LINE__,dcrudArguments_getLong     ( args, "long-key"     , &int64Value          ))
   PRT( PRId64, int64Value      , LONG_VALUE );
   CHK(__FILE__,__LINE__,dcrudArguments_getUlong    ( args, "ulong-key"    , &uint64Value         ))
   PRT( PRIu64, uint64Value     , ULONG_VALUE );
   CHK(__FILE__,__LINE__,dcrudArguments_getFloat    ( args, "float-key"    , &floatValue          ))
   PRT( "f", floatValue         , FLOAT_VALUE );
   CHK(__FILE__,__LINE__,dcrudArguments_getDouble   ( args, "double-key"   , &doubleValue         ))
   PRT( "f", doubleValue        , DOUBLE_VALUE );
   CHK(__FILE__,__LINE__,dcrudArguments_getString   ( args, "string-key"   , buffer, sizeof( buffer )))
   printf( "%-4s: String == '%s'\n", ( 0 == strcmp( buffer, STRING_VALUE )) ? OK : FAIL, STRING_VALUE );
   CHK(__FILE__,__LINE__,dcrudArguments_getClassID  ( args, "class-id-key" , &dcrudClassIDValue   ))
   PRT( "p", dcrudClassIDValue  , (void *)CLASS_ID_VALUE );
   CHK(__FILE__,__LINE__,dcrudArguments_getGUID     ( args, "guid-key"     , &dcrudGUIDValue      ))
   PRT( "p", dcrudGUIDValue     , (void *)GUID_VALUE );
   CHK(__FILE__,__LINE__,dcrudArguments_getShareable( args, "shareable-key", &dcrudShareableValue ))
   PRT( "p", dcrudShareableValue, (void *)SHAREABLE_VALUE );
   CHK(__FILE__,__LINE__,dcrudArguments_getType     ( args, "boolean-key"  , &dcrudTypeValue      ))
   PRT( "d", dcrudTypeValue, dcrudTYPE_BOOLEAN );
   CHK(__FILE__,__LINE__,dcrudArguments_getType     ( args, "byte-key"     , &dcrudTypeValue      ))
   PRT( "d", dcrudTypeValue, dcrudTYPE_BYTE );
   CHK(__FILE__,__LINE__,dcrudArguments_getType     ( args, "short-key"    , &dcrudTypeValue      ))
   PRT( "d", dcrudTypeValue, dcrudTYPE_SHORT );
   CHK(__FILE__,__LINE__,dcrudArguments_getType     ( args, "ushort-key"   , &dcrudTypeValue      ))
   PRT( "d", dcrudTypeValue, dcrudTYPE_UNSIGNED_SHORT );
   CHK(__FILE__,__LINE__,dcrudArguments_getType     ( args, "int-key"      , &dcrudTypeValue      ))
   PRT( "d", dcrudTypeValue, dcrudTYPE_INTEGER );
   CHK(__FILE__,__LINE__,dcrudArguments_getType     ( args, "uint-key"     , &dcrudTypeValue      ))
   PRT( "d", dcrudTypeValue, dcrudTYPE_UNSIGNED_INTEGER );
   CHK(__FILE__,__LINE__,dcrudArguments_getType     ( args, "long-key"     , &dcrudTypeValue      ))
   PRT( "d", dcrudTypeValue, dcrudTYPE_LONG );
   CHK(__FILE__,__LINE__,dcrudArguments_getType     ( args, "ulong-key"    , &dcrudTypeValue      ))
   PRT( "d", dcrudTypeValue, dcrudTYPE_UNSIGNED_LONG );
   CHK(__FILE__,__LINE__,dcrudArguments_getType     ( args, "float-key"    , &dcrudTypeValue      ))
   PRT( "d", dcrudTypeValue, dcrudTYPE_FLOAT );
   CHK(__FILE__,__LINE__,dcrudArguments_getType     ( args, "double-key"   , &dcrudTypeValue      ))
   PRT( "d", dcrudTypeValue, dcrudTYPE_DOUBLE );
   CHK(__FILE__,__LINE__,dcrudArguments_getType     ( args, "string-key"   , &dcrudTypeValue      ))
   PRT( "d", dcrudTypeValue, dcrudTYPE_STRING );
   CHK(__FILE__,__LINE__,dcrudArguments_getType     ( args, "class-id-key" , &dcrudTypeValue      ))
   PRT( "d", dcrudTypeValue, dcrudTYPE_CLASS_ID );
   CHK(__FILE__,__LINE__,dcrudArguments_getType     ( args, "guid-key"     , &dcrudTypeValue      ))
   PRT( "d", dcrudTypeValue, dcrudTYPE_GUID );
   CHK(__FILE__,__LINE__,dcrudArguments_getType     ( args, "shareable-key", &dcrudTypeValue      ))
   PRT( "d", dcrudTypeValue, dcrudTYPE_SHAREABLE );
   /*CHK(__FILE__,__LINE__,dcrudArguments_serialize   ( args, ioByteBuffer target ))*/
   CHK(__FILE__,__LINE__,dcrudArguments_clear       ( args ))
   CHK(__FILE__,__LINE__,dcrudArguments_isEmpty     ( args, &isEmpty ))
   printf( "%s: isEmpty\n", isEmpty ? "OK  " : "FAIL" );
   CHK(__FILE__,__LINE__,dcrudArguments_delete( &args ))
   CHK(__FILE__,__LINE__,dcrudClassID_delete( &CLASS_ID_VALUE ))
   CHK(__FILE__,__LINE__,dcrudGUID_delete( &GUID_VALUE ))
   return UTIL_STATUS_NO_ERROR;
}
