#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

typedef enum utilStatus_e {

   UTIL_STATUS_NO_ERROR = 0,

   /* Lexicographic order */
   UTIL_STATUS_BAD_CAST,
   UTIL_STATUS_DUPLICATE,
   UTIL_STATUS_ILLEGAL_STATE,
   UTIL_STATUS_NOT_FOUND,
   UTIL_STATUS_NULL_ARGUMENT,
   UTIL_STATUS_OUT_OF_RANGE,
   UTIL_STATUS_OVERFLOW,  /* relative to put operations */
   UTIL_STATUS_STD_API_ERROR,
   UTIL_STATUS_TOO_MANY,
   UTIL_STATUS_TYPE_MISMATCH,
   UTIL_STATUS_UNDERFLOW, /* relative to get operations */

   UTIL_STATUS_LAST

} utilStatus;

extern const char * utilStatusMessages[UTIL_STATUS_LAST+1];

extern utilStatus utilStatus_checkAndLog(
   utilStatus   This,
   const char * file,
   int          line,
   const char * format,
   ...                  );

#define CHK( FILE, LINE, EXPRESSION ) {\
   utilStatus status = EXPRESSION;\
   utilStatus_checkAndLog( status, FILE, LINE, # EXPRESSION );\
   if( UTIL_STATUS_NO_ERROR != status ) {\
      return status;\
   }\
}

#ifdef __cplusplus
}
#endif
