#pragma once

typedef enum osStatus_e {

   OS_STATUS_NO_ERROR,

   OS_STATUS_LAST

} osStatus;

extern const char * osStatusMessages[OS_STATUS_LAST];
