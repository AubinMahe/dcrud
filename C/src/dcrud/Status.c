#include <dcrud/Status.h>

const char * dcrudStatusMessages[DCRUD_STATUS_LAST] = {
/* DCRUD_NO_ERROR          */"No error",
/* DCRUD_ALREADY_CREATED   */"This Shareable has already been published",
/* DCRUD_NOT_OWNER         */"This cache isn't the owner of this Shareable",
/* DCRUD_NOT_CREATED       */"This Shareable has never been published",
/* DCRUD_NOT_IN_THIS_CACHE */"This Shareable doesn't exists in this cache",
/* DCRUD_TOO_MANY_CACHES   */"Too many cache created",
};
