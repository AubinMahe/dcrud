#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <util/Status.h>

typedef void * collForeachResult;

typedef struct collForeach_s {

   unsigned int      index;
   const void *      key;
   void *            value;
   void *            user;
   collForeachResult retVal;

} collForeach;

typedef utilStatus( * collForeachFunction )( collForeach * context );

#ifdef __cplusplus
}
#endif
