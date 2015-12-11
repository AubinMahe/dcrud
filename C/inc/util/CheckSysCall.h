#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <util/types.h>

extern bool utilCheckSysCall(
   bool         ok,
   const char * file,
   int          line,
   const char * format, ... );

#ifdef __cplusplus
}
#endif
