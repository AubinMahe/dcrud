#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "ISO8601Date.h"
#include "Types.h"

typedef struct Person__ {

   channelFactory *       factory;
   channelTestForname     forname;
   channelTestName        name;
   channelTestISO8601Date birthdate;

} channelTestPerson;

utilStatus channelTestPerson_registerFactory();
utilStatus channelTestPerson_getFactory( channelFactory ** target );

utilStatus channelTestPerson_new     ( /* */ channelTestPerson ** This );
utilStatus channelTestPerson_encode  ( const channelTestPerson *  This, ioByteBuffer encoder );
utilStatus channelTestPerson_decode  ( /* */ channelTestPerson *  This, ioByteBuffer decoder );
utilStatus channelTestPerson_init    ( /* */ channelTestPerson *  This );
utilStatus channelTestPerson_toString( const channelTestPerson *  This, char * target, size_t * size );
utilStatus channelTestPerson_delete  ( /* */ channelTestPerson ** This );

#ifdef __cplusplus
}
#endif
