#pragma once
#include <stdlib.h>
#include <stdint.h>

#ifndef __cplusplus
   typedef int bool;
#  define false 0
#  define true (!false)
#endif

typedef unsigned char byte;
#ifdef _WIN32
typedef          __int64   int64_t;
typedef unsigned __int64   uint64_t;
#endif

#define UTIL_ADT(T) typedef struct T##_s { int unused; } * T
