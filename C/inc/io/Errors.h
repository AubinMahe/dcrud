#pragma once

typedef enum ioError_e {

   ioError_NO_ERROR,
   ioError_OVERFLOW,  /* relative to put operations */
   ioError_UNDERFLOW, /* relative to get operations */

   ioError_ERROR_LAST
} ioError;

extern const char * ioErrorMessages[ioError_ERROR_LAST];

