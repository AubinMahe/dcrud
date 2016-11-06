#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <dcrud/Shareable.h>
#include <dcrud/IRequired.h>

UTIL_ADT( dcrudArguments );

utilStatus dcrudArguments_new         ( dcrudArguments * This );
utilStatus dcrudArguments_delete      ( dcrudArguments * This );
utilStatus dcrudArguments_clear       ( dcrudArguments This );
utilStatus dcrudArguments_setMode     ( dcrudArguments This, dcrudCallMode mode );
utilStatus dcrudArguments_setQueue    ( dcrudArguments This, dcrudQueueIndex queue );
utilStatus dcrudArguments_serialize   ( dcrudArguments This, ioByteBuffer target );

utilStatus dcrudArguments_isEmpty     ( dcrudArguments This, bool * isEmpty );
utilStatus dcrudArguments_getCount    ( dcrudArguments This, unsigned int * count );
utilStatus dcrudArguments_getMode     ( dcrudArguments This, dcrudCallMode * mode );
utilStatus dcrudArguments_getQueue    ( dcrudArguments This, dcrudQueueIndex * queue );
utilStatus dcrudArguments_dump        ( dcrudArguments This, FILE * target );

utilStatus dcrudArguments_putNull     ( dcrudArguments This, const char * key );
utilStatus dcrudArguments_putBoolean  ( dcrudArguments This, const char * key, bool value );
utilStatus dcrudArguments_putByte     ( dcrudArguments This, const char * key, byte value );
utilStatus dcrudArguments_putChar     ( dcrudArguments This, const char * key, char value );
utilStatus dcrudArguments_putShort    ( dcrudArguments This, const char * key, short value );
utilStatus dcrudArguments_putUshort   ( dcrudArguments This, const char * key, unsigned short value );
utilStatus dcrudArguments_putInt      ( dcrudArguments This, const char * key, int value );
utilStatus dcrudArguments_putUint     ( dcrudArguments This, const char * key, unsigned int value );
utilStatus dcrudArguments_putLong     ( dcrudArguments This, const char * key, int64_t value );
utilStatus dcrudArguments_putUlong    ( dcrudArguments This, const char * key, uint64_t value );
utilStatus dcrudArguments_putFloat    ( dcrudArguments This, const char * key, float value );
utilStatus dcrudArguments_putDouble   ( dcrudArguments This, const char * key, double value );
utilStatus dcrudArguments_putString   ( dcrudArguments This, const char * key, const char * value );
utilStatus dcrudArguments_putClassID  ( dcrudArguments This, const char * key, dcrudClassID value );
utilStatus dcrudArguments_putGUID     ( dcrudArguments This, const char * key, dcrudGUID value );
utilStatus dcrudArguments_putShareable( dcrudArguments This, const char * key, dcrudShareable value );

utilStatus dcrudArguments_isNull      ( dcrudArguments This, const char * key, bool * isNull );
utilStatus dcrudArguments_getType     ( dcrudArguments This, const char * key, dcrudType * type );
utilStatus dcrudArguments_getBoolean  ( dcrudArguments This, const char * key, bool * value );
utilStatus dcrudArguments_getByte     ( dcrudArguments This, const char * key, byte * value );
utilStatus dcrudArguments_getChar     ( dcrudArguments This, const char * key, char * value );
utilStatus dcrudArguments_getShort    ( dcrudArguments This, const char * key, short * value );
utilStatus dcrudArguments_getUshort   ( dcrudArguments This, const char * key, unsigned short * value );
utilStatus dcrudArguments_getInt      ( dcrudArguments This, const char * key, int * value );
utilStatus dcrudArguments_getUint     ( dcrudArguments This, const char * key, unsigned int * value );
utilStatus dcrudArguments_getLong     ( dcrudArguments This, const char * key, int64_t * value );
utilStatus dcrudArguments_getUlong    ( dcrudArguments This, const char * key, uint64_t * value );
utilStatus dcrudArguments_getFloat    ( dcrudArguments This, const char * key, float * value );
utilStatus dcrudArguments_getDouble   ( dcrudArguments This, const char * key, double * value );
utilStatus dcrudArguments_getString   ( dcrudArguments This, const char * key, char * target, size_t targetSize );
utilStatus dcrudArguments_getClassID  ( dcrudArguments This, const char * key, dcrudClassID * value );
utilStatus dcrudArguments_getGUID     ( dcrudArguments This, const char * key, dcrudGUID * value );
utilStatus dcrudArguments_getShareable( dcrudArguments This, const char * key, dcrudShareable * value );

#ifdef __cplusplus
}
#endif
