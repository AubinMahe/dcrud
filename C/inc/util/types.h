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

#define UTIL_CONCAT_(x,y) x ## y
#define UTIL_CONCAT(x,y)  UTIL_CONCAT_(x,y)
#define UTIL_ADT(T) typedef struct UTIL_CONCAT(T,_s) { int unused; } * T
