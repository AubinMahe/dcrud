#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <winsock2.h>
#  include <ws2tcpip.h>
#  include <mswsock.h>
   int inet_pton( int af, const char * src, void * dst );
#else
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <netinet/ip.h>
#  include <arpa/inet.h>
#  include <netdb.h>
   typedef int SOCKET;
#  define INVALID_SOCKET (-1)
#  define closesocket close
#endif

#ifdef __cplusplus
}
#endif
