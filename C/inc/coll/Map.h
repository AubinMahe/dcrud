#pragma once

#include <util/types.h>

#define COLL_MAP_TYPE collMap
typedef void * UTIL_CONCAT(COLL_MAP_TYPE,Value);

#include <coll/Map.template>
#undef COLL_MAP_TYPE
#undef COLL_MAP_NAME
