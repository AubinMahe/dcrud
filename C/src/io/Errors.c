#include <io/Status.h>

const char * ioStatusMessages[IO_STATUS_LAST] = {
   "No error",

   "Too many put operation or buffer too small",
   "Too many get operation or buffer too small",
   "reset() called without previous call to mark()",
   "sendto failed, see perror()",
   "recv failed, see perror()",
};
