#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <util/Status.h>

utilStatus ioNetworkInterfaces_getFirst(
   bool     multicastCapabilities,
   char *   host,
   unsigned hostSize );

#ifdef __cplusplus
}
#endif
