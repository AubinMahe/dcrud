#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <dcrud/ClassID.h>

typedef struct dcrudClassIDImpl_s {

   unsigned magic;
   byte     package_1;
   byte     package_2;
   byte     package_3;
   byte     clazz;

} dcrudClassIDImpl;

UTIL_DECLARE_SAFE_CAST( dcrudClassID );

#ifdef __cplusplus
}
#endif
