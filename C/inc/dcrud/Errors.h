#pragma once

typedef enum dcrudErrorCode_e {

   DCRUD_NO_ERROR = 0,
   DCRUD_ALREADY_CREATED,
   DCRUD_NOT_CREATED,
   DCRUD_NOT_IN_THIS_REPOSITORY,

   DCRUD_ERROR_LAST
} dcrudErrorCode;

extern const char * dcrudErrorMessages[DCRUD_ERROR_LAST];
