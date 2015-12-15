#pragma once

#ifdef NO_ERROR
#  undef NO_ERROR
#endif

namespace dcrud {

   enum Status {

      NO_ERROR,
      ALREADY_CREATED,
      NOT_OWNER,
      NOT_CREATED,
      NOT_IN_THIS_CACHE
   };
}
