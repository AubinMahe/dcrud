#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <channel/Factories.h>
#include <channel/CoDec.h>

typedef struct channelTestISO8601Date__ {

   channelFactory * factory;
   unsigned char    day;
   unsigned char    month;
   unsigned short   year;

} channelTestISO8601Date;

utilStatus channelTestISO8601Date_registerFactory( void );
utilStatus channelTestISO8601Date_getFactory( channelFactory ** target );

utilStatus channelTestISO8601Date_new     ( /* */ channelTestISO8601Date ** This );
utilStatus channelTestISO8601Date_encode  ( const channelTestISO8601Date *  This, ioByteBuffer encoder );
utilStatus channelTestISO8601Date_decode  ( /* */ channelTestISO8601Date *  This, ioByteBuffer decoder );
utilStatus channelTestISO8601Date_init    ( /* */ channelTestISO8601Date *  This );
utilStatus channelTestISO8601Date_isValid ( const channelTestISO8601Date *  This );
utilStatus channelTestISO8601Date_parse   ( /* */ channelTestISO8601Date *  This, const char * src );
utilStatus channelTestISO8601Date_format  ( const channelTestISO8601Date *  This, char * target, size_t   size );
utilStatus channelTestISO8601Date_toString( const channelTestISO8601Date *  This, char * target, size_t * size );
utilStatus channelTestISO8601Date_delete  ( /* */ channelTestISO8601Date ** This );

#ifdef __cplusplus
}
#endif
