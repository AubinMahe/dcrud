#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <dcrud/Shareable.h>

UTIL_ADT( dcrudArguments );

dcrudArguments dcrudArguments_new         ( void );
void           dcrudArguments_delete      ( dcrudArguments * This );
void           dcrudArguments_clear       ( dcrudArguments This );
bool           dcrudArguments_isEmpty     ( dcrudArguments This );
unsigned int   dcrudArguments_getCount    ( dcrudArguments This );
void           dcrudArguments_setMode     ( dcrudArguments This, byte mode );
void           dcrudArguments_setQueue    ( dcrudArguments This, byte queue );
void           dcrudArguments_putNull     ( dcrudArguments This, const char * key );
void           dcrudArguments_putByte     ( dcrudArguments This, const char * key, byte             value );
void           dcrudArguments_putBoolean  ( dcrudArguments This, const char * key, bool             value );
void           dcrudArguments_putShort    ( dcrudArguments This, const char * key, unsigned short   value );
void           dcrudArguments_putInt      ( dcrudArguments This, const char * key, unsigned int     value );
void           dcrudArguments_putLong     ( dcrudArguments This, const char * key, uint64_t         value );
void           dcrudArguments_putFloat    ( dcrudArguments This, const char * key, float            value );
void           dcrudArguments_putDouble   ( dcrudArguments This, const char * key, double           value );
void           dcrudArguments_putString   ( dcrudArguments This, const char * key, const char *     value );
void           dcrudArguments_putClassID  ( dcrudArguments This, const char * key, dcrudClassID     value );
void           dcrudArguments_putGUID     ( dcrudArguments This, const char * key, dcrudGUID        value );
void           dcrudArguments_putShareable( dcrudArguments This, const char * key, dcrudShareable   value );
bool           dcrudArguments_isNull      ( dcrudArguments This, const char * key );
bool           dcrudArguments_getByte     ( dcrudArguments This, const char * key, byte *           value );
bool           dcrudArguments_getBoolean  ( dcrudArguments This, const char * key, bool *           value );
bool           dcrudArguments_getShort    ( dcrudArguments This, const char * key, unsigned short * value );
bool           dcrudArguments_getInt      ( dcrudArguments This, const char * key, unsigned int *   value );
bool           dcrudArguments_getLong     ( dcrudArguments This, const char * key, uint64_t *       value );
bool           dcrudArguments_getFloat    ( dcrudArguments This, const char * key, float *          value );
bool           dcrudArguments_getDouble   ( dcrudArguments This, const char * key, double *         value );
bool           dcrudArguments_getString   ( dcrudArguments This, const char * key, char * buffer, unsigned int capacity );
bool           dcrudArguments_getClassID  ( dcrudArguments This, const char * key, dcrudClassID *   value );
bool           dcrudArguments_getGUID     ( dcrudArguments This, const char * key, dcrudGUID *      value );
bool           dcrudArguments_getShareable( dcrudArguments This, const char * key, dcrudShareable * value );
dcrudType      dcrudArguments_getType     ( dcrudArguments This, const char * key );
bool           dcrudArguments_serialize   ( dcrudArguments This, ioByteBuffer target );
void           dcrudArguments_dump        ( dcrudArguments This, FILE * target );

#ifdef __cplusplus
}
#endif
