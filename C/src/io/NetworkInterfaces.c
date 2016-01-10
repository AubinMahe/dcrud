#include <io/NetworkInterfaces.h>

#include <net/if.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char * ioNetworkInterfaces_getFirst( bool multicastCapabilities ) {
   static char      host[NI_MAXHOST];
   struct ifaddrs * ifaddr;
   struct ifaddrs * ifa;
   int              n;

   memset( host, 0, sizeof( host ));
   if( getifaddrs( &ifaddr ) == -1 ) {
      perror( "getifaddrs" );
      exit( EXIT_FAILURE );
   }
   for( ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, ++n ) {
      if( ifa->ifa_addr ) {
         int family = ifa->ifa_addr->sa_family;
         if(( family == AF_INET )||( family == AF_INET6 )) {
            const socklen_t len       = ( family == AF_INET ) ? sizeof( struct sockaddr_in ) : sizeof( struct sockaddr_in6 );
            bool            up        = ( ifa->ifa_flags & IFF_UP        );
            bool            loopback  = ( ifa->ifa_flags & IFF_LOOPBACK  );
            bool            multicast = ( ifa->ifa_flags & IFF_MULTICAST );
            int             s         =
               getnameinfo( ifa->ifa_addr, len, host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST );
            if( s ) {
               printf( "getnameinfo() failed: %s\n", gai_strerror( s ));
               exit( EXIT_FAILURE );
            }
            if(      up
               &&  ! loopback
               &&( ! multicastCapabilities || ( multicastCapabilities && multicast )))
            {
               break;
            }
         }
      }
   }
   freeifaddrs( ifaddr );
   return host;
}
