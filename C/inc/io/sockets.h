#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <winsock2.h>
#  include <ws2tcpip.h>
#  include <mswsock.h>
#  define SHUT_RD       SD_RECEIVE
#  define SHUT_WR       SD_SEND
#  define SHUT_RDWR     SD_BOTH
   int inet_pton( int af, const char * src, void * dst );
#else
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <netinet/ip.h>
#  include <arpa/inet.h>
#  include <netdb.h>
#  include <unistd.h>
   typedef int SOCKET;
#  define INVALID_SOCKET (-1)
#  define closesocket close
#endif

#include <util/Status.h>

utilStatus ioOpenBindAndConnectDatagramSocket(
   const char *   intrfc,
   unsigned short localPort,
   const char *   remoteAddress,
   unsigned short remotePort,
   SOCKET *       target        );

#ifdef __cplusplus
}
#endif
