#pragma once
#ifdef __cplusplus
extern "C" {
#endif

typedef void * collForeachResult;

typedef struct collForeach_s {

   unsigned int      index;
   void *            item;
   void *            user;
   collForeachResult retVal;

} collForeach;

typedef bool( * collForeachFunction )( collForeach * context );

#ifdef __cplusplus
}
#endif
