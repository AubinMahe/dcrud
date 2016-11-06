#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "Person.h"
#include <channel/List.h>

typedef struct Family__ {

   channelFactory *   factory;
   channelTestPerson   parent1;
   channelTestPerson * parent2;
   channelDECL_LIST( channelTestPerson, children );

} channelTestFamily;

utilStatus channelTestFamily_registerFactory( void );
utilStatus channelTestFamily_getFactory( channelFactory ** target );

utilStatus channelTestFamily_new     ( /* */ channelTestFamily ** This );
utilStatus channelTestFamily_encode  ( const channelTestFamily *  This, ioByteBuffer encoder );
utilStatus channelTestFamily_decode  ( /* */ channelTestFamily *  This, ioByteBuffer decoder );
utilStatus channelTestFamily_init    ( /* */ channelTestFamily *  This );
utilStatus channelTestFamily_done    ( /* */ channelTestFamily *  This );
utilStatus channelTestFamily_toString( const channelTestFamily *  This, char * target, size_t * size );
utilStatus channelTestFamily_delete  ( /* */ channelTestFamily ** This );

#ifdef __cplusplus
}
#endif
