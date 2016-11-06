#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <dcrud/IParticipant.h>

typedef struct dcrudShareableImpl_s {

   unsigned            magic;
   dcrudGUID           id;
   dcrudClassID        classID;
   dcrudLocalFactory * factory;
   dcrudShareableData  data;

} dcrudShareableImpl;

UTIL_DECLARE_SAFE_CAST(dcrudShareable);

#ifdef __cplusplus
}
#endif
