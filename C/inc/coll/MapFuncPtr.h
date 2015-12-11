#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <util/types.h>

#undef  COLL_MAP_TYPE
#define COLL_MAP_TYPE collMapFuncPtr
typedef void (* UTIL_CONCAT(COLL_MAP_TYPE,Value))( void * );

#include <coll/Map.template>
#undef COLL_MAP_TYPE
#undef COLL_MAP_NAME

#ifdef __cplusplus
}
#endif
