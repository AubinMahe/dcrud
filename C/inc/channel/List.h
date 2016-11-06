#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <util/Status.h>

typedef struct channelList_s {

   unsigned int count;
   unsigned int maxCount;
   void **      items;

} channelList;

utilStatus channelList_init   ( channelList * list );
utilStatus channelList_done   ( channelList * list );
utilStatus channelList_addItem( channelList * list, void * item );

#define channelDECL_LIST( TYPE, ATTR )\
   struct {\
      unsigned count;\
      unsigned maxCount;\
      TYPE **  items;\
   } ATTR

#ifdef __cplusplus
}
#endif
