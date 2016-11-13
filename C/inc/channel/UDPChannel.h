#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <io/ByteBuffer.h>

UTIL_ADT( channelUDPChannel );

typedef utilStatus (* channelMsgConsumerFn)( short msgId, ioByteBuffer decoder, void * userData );

utilStatus channelUDPChannel_new       ( channelUDPChannel * This, const char * intrfc, unsigned short local, const char * host, unsigned short remote );
utilStatus channelUDPChannel_send      ( channelUDPChannel   This, ioByteBuffer encoder );
utilStatus channelUDPChannel_addHandler( channelUDPChannel   This, short id, channelMsgConsumerFn handler, void * userContext );
utilStatus channelUDPChannel_delete    ( channelUDPChannel * This );

#ifdef __cplusplus
}
#endif
