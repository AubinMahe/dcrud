#include <io/Errors.h>

const char * ioErrorMessages[ioError_ERROR_LAST] = {
   "No error",

   "Too many put operation or buffer too small",
   "Too many get operation or buffer too small",
};
