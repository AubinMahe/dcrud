#pragma once
#ifdef __cplusplus
extern "C" {
#else
#endif

#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

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

#define UTIL_ADT(T)\
   typedef struct T ## _s { int unused; } * T

#define UTIL_DECLARE_SAFE_CAST(T)\
   T ## Impl * T ##_safeCast( T self, utilStatus * status )

#define UTIL_DEFINE_SAFE_CAST(T)\
   T ## Impl * T ##_safeCast( T self, utilStatus * status ) {\
      T ## Impl * retVal = NULL;\
      *status = UTIL_STATUS_NO_ERROR;\
      if( self == NULL ) {\
         *status = UTIL_STATUS_NULL_ARGUMENT;\
      }\
      else {\
         retVal = (T ## Impl *)self;\
         if( retVal->magic != T ## ImplMAGIC ) {\
            *status = UTIL_STATUS_BAD_CAST;\
            retVal = NULL;\
         }\
      }\
      return retVal;\
   }

#ifdef __cplusplus
}
#endif
