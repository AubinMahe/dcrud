#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#define _GNU_SOURCE
#define __USE_XOPEN
#include <time.h>
#undef _GNU_SOURCE
#undef __USE_XOPEN

#ifdef _WIN32
char * strptime( const char * s, const char * format, struct tm * tm );
#endif

#ifdef __cplusplus
}
#endif
