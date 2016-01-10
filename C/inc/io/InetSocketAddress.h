#pragma once
#ifdef __cplusplus
extern "C" {
#endif

typedef struct ioInetSocketAddress_s {

   char           inetAddress[16];
   unsigned short port;

} ioInetSocketAddress;

void ioInetSocketAddress_init( ioInetSocketAddress * This, const char * host, unsigned short port );

ioInetSocketAddress * ioInetSocketAddress_new( const char * host, unsigned short port );

int ioInetSocketAddress_comparator(
   const ioInetSocketAddress * left,
   const ioInetSocketAddress * right );

#ifdef __cplusplus
}
#endif
