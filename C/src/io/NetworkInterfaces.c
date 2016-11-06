#include <io/NetworkInterfaces.h>

#include <net/if.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

utilStatus ioNetworkInterfaces_getFirst(
   bool     multicastCapabilities,
   char *   host,
   unsigned hostSize )
{
   struct ifaddrs * ifaddr;
   struct ifaddrs * ifa;
   bool             notFound;

   if( NULL == host ) {
      return UTIL_STATUS_NULL_ARGUMENT;
   }
   memset( host, 0, hostSize );
   if( getifaddrs( &ifaddr ) == -1 ) {
      return UTIL_STATUS_STD_API_ERROR;
   }
   for( ifa = ifaddr, notFound; ifa != NULL; ifa = ifa->ifa_next ) {
      if( ifa->ifa_addr ) {
         int family = ifa->ifa_addr->sa_family;
         if(( family == AF_INET )||( family == AF_INET6 )) {
            const socklen_t len = ( family == AF_INET )
               ? sizeof( struct sockaddr_in )
               : sizeof( struct sockaddr_in6 );
            bool up        = ( ifa->ifa_flags & IFF_UP        );
            bool loopback  = ( ifa->ifa_flags & IFF_LOOPBACK  );
            bool multicast = ( ifa->ifa_flags & IFF_MULTICAST );
            if(      up
               &&  ! loopback
               &&( ! multicastCapabilities || ( multicastCapabilities && multicast )))
            {
               if( getnameinfo( ifa->ifa_addr, len, host, hostSize, NULL, 0, NI_NUMERICHOST )) {
                  return UTIL_STATUS_STD_API_ERROR;
               }
               notFound = false;
            }
         }
      }
   }
   freeifaddrs( ifaddr );
   return UTIL_STATUS_NO_ERROR;
}
