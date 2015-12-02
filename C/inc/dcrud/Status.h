#pragma once

typedef enum dcrudStatus_e {

   DCRUD_NO_ERROR = 0,
   DCRUD_INIT_FAILED,
   DCRUD_ALREADY_CREATED,
   DCRUD_NOT_OWNER,
   DCRUD_NOT_CREATED,
   DCRUD_NOT_IN_THIS_CACHE,
   DCRUD_TOO_MANY_CACHES,

   DCRUD_STATUS_LAST

} dcrudStatus;

extern const char * dcrudStatusMessages[DCRUD_STATUS_LAST];
