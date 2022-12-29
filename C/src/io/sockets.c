#include "io/sockets.h"
#include <string.h>

#ifdef WIN32

int inet_pton( int af, const char * src, void * dst ) {
   (void)af;
   (void)src;
   (void)dst;
   return 1;
}

#endif

utilStatus ioOpenBindAndConnectDatagramSocket(
   const char *   intrfc,
   unsigned short localPort,
   const char *   remoteAddress,
   unsigned short remotePort,
   SOCKET *       sckt        )
{
   struct sockaddr_in localAddr;
   struct hostent *   he = NULL;
   struct sockaddr_in target;

   if( sckt == NULL ) {
      return UTIL_STATUS_NULL_ARGUMENT;
   }
   *sckt = socket( AF_INET, SOCK_DGRAM, 0 );
   memset( &localAddr, 0, sizeof( localAddr ));
   localAddr.sin_family = AF_INET;
   inet_pton( AF_INET, intrfc, &localAddr.sin_addr.s_addr );
   localAddr.sin_port = htons( localPort );
   if( bind( *sckt, (struct sockaddr *)&localAddr, sizeof( localAddr ))) {
      closesocket( *sckt );
      return UTIL_STATUS_STD_API_ERROR;
   }
   he = gethostbyname( remoteAddress );
   if( ! he ) {
      closesocket( *sckt );
      return 3;
   }
   target.sin_family = AF_INET;
   target.sin_port   = htons( remotePort );
   memcpy( &target.sin_addr, he->h_addr_list[0], (size_t)he->h_length );
   if( connect( *sckt, (struct sockaddr *)&target, sizeof( target ))) {
      closesocket( *sckt );
      return UTIL_STATUS_STD_API_ERROR;
   }
   return UTIL_STATUS_NO_ERROR;
}
