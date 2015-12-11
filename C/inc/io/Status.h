#pragma once
#ifdef __cplusplus
extern "C" {
#endif

typedef enum ioStatus_e {

   IO_STATUS_NO_ERROR,
   IO_STATUS_OVERFLOW,        /* relative to put operations */
   IO_STATUS_UNDERFLOW,       /* relative to get operations */
   IO_STATUS_NO_MARK,         /* reset() called without previous call to mark() */
   IO_STATUS_SENDTO_FAILED,
   IO_STATUS_RECV_FAILED,

   IO_STATUS_LAST

} ioStatus;

extern const char * ioStatusMessages[IO_STATUS_LAST];

#ifdef __cplusplus
}
#endif
