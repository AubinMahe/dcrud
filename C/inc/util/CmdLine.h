#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "Status.h"

UTIL_ADT(utilCmdLine);

utilStatus utilCmdLine_new       ( /* */ utilCmdLine * This );
utilStatus utilCmdLine_delete    ( /* */ utilCmdLine * This );
utilStatus utilCmdLine_addBoolean( /* */ utilCmdLine   This, const char * key, bool dflt );
utilStatus utilCmdLine_addChar   ( /* */ utilCmdLine   This, const char * key, char dflt );
utilStatus utilCmdLine_addByte   ( /* */ utilCmdLine   This, const char * key, unsigned char dflt );
utilStatus utilCmdLine_addShort  ( /* */ utilCmdLine   This, const char * key, short dflt );
utilStatus utilCmdLine_addUShort ( /* */ utilCmdLine   This, const char * key, unsigned short dflt );
utilStatus utilCmdLine_addInt    ( /* */ utilCmdLine   This, const char * key, int dflt );
utilStatus utilCmdLine_addUInt   ( /* */ utilCmdLine   This, const char * key, unsigned int dflt );
utilStatus utilCmdLine_addFloat  ( /* */ utilCmdLine   This, const char * key, float dflt );
utilStatus utilCmdLine_addDouble ( /* */ utilCmdLine   This, const char * key, double dflt );
utilStatus utilCmdLine_addString ( /* */ utilCmdLine   This, const char * key, const char * dflt );
utilStatus utilCmdLine_parse     ( /* */ utilCmdLine   This, int argc, char * argv[] );
utilStatus utilCmdLine_getBoolean( const utilCmdLine   This, const char * key, bool * result );
utilStatus utilCmdLine_getChar   ( const utilCmdLine   This, const char * key, char * result );
utilStatus utilCmdLine_getByte   ( const utilCmdLine   This, const char * key, byte * result );
utilStatus utilCmdLine_getShort  ( const utilCmdLine   This, const char * key, short * result );
utilStatus utilCmdLine_getUShort ( const utilCmdLine   This, const char * key, unsigned short * result );
utilStatus utilCmdLine_getInt    ( const utilCmdLine   This, const char * key, int * result );
utilStatus utilCmdLine_getUInt   ( const utilCmdLine   This, const char * key, unsigned int * result );
utilStatus utilCmdLine_getFloat  ( const utilCmdLine   This, const char * key, float * result );
utilStatus utilCmdLine_getDouble ( const utilCmdLine   This, const char * key, double * result );
utilStatus utilCmdLine_getString ( const utilCmdLine   This, const char * key, const char ** result );

#ifdef __cplusplus
}
#endif
