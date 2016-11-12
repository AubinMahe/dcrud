#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <util/Status.h>

typedef struct collForeach_s {

   unsigned int      index;
   const void *      key;
   void *            value;
   void *            user;

} collForeach;

typedef utilStatus( * collForeachFunction )( collForeach * context );

#ifdef __cplusplus
}
#endif
