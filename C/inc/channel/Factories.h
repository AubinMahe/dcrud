#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <io/ByteBuffer.h>

typedef int        (* channelGetClassID)( void );
typedef utilStatus (* channelCreate    )( /* */ void ** This );
typedef utilStatus (* channelEncode    )( const void *  This, ioByteBuffer encoder );
typedef utilStatus (* channelDecode    )( /* */ void *  This, ioByteBuffer decoder );

typedef struct channelFactory_s {

   channelGetClassID getClassID;
   channelCreate     create;
   channelEncode     encode;
   channelDecode     decode;

} channelFactory;

utilStatus channelFactories_add ( channelFactory * factory );
utilStatus channelFactories_get ( int id, channelFactory ** result );
utilStatus channelFactories_done( void );

#ifdef __cplusplus
}
#endif
