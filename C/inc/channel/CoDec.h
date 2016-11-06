#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <io/ByteBuffer.h>

utilStatus channelCoDec_init        ( ioByteBuffer encoder, short id );
utilStatus channelCoDec_putReference( ioByteBuffer encoder, const void * source );
utilStatus channelCoDec_getReference( ioByteBuffer decoder, void * target );
utilStatus channelCoDec_putValue    ( ioByteBuffer encoder, const void * source );
utilStatus channelCoDec_getValue    ( ioByteBuffer decoder, void * target );
utilStatus channelCoDec_putArray    ( ioByteBuffer encoder, const void * source, unsigned count );
utilStatus channelCoDec_getArray    ( ioByteBuffer decoder, void * target, unsigned count );
utilStatus channelCoDec_putList     ( ioByteBuffer encoder, const void * source );
utilStatus channelCoDec_getList     ( ioByteBuffer decoder, void * target );

#ifdef __cplusplus
}
#endif
