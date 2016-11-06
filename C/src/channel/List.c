#include <channel/List.h>
#include "poolSizes.h"
#include <util/Pool.h>

#include <stdlib.h>
#include <string.h>

#ifdef STATIC_ALLOCATION
typedef void * channelListItems[channelListItems_POOL_SIZE];
UTIL_POOL_DECLARE( channelListItems );
#endif

utilStatus channelList_init( channelList * list ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( NULL == list ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      UTIL_POOL_INIT( channelListItems );
      if( UTIL_STATUS_NO_ERROR == status ) {
         memset( list, 0, sizeof( channelList ));
#ifdef STATIC_ALLOCATION
         status = utilPool_reserve( &channelListItemsPool, &list->items );
         if( UTIL_STATUS_NO_ERROR == status ) {
            list->maxCount = channelListItems_POOL_SIZE;
         }
#endif
      }
   }
   return status;
}

utilStatus channelList_done( channelList * list ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
#ifdef STATIC_ALLOCATION
   status = utilPool_release( &channelListItemsPool, &list->items );
#else
   if( list->items ) {
      free( list->items );
   }
#endif
   return status;
}

utilStatus channelList_addItem( channelList * list, void * item ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( NULL == list || NULL == item ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      if( list->count == list->maxCount ) {
#ifdef STATIC_ALLOCATION
         status = UTIL_STATUS_TOO_MANY;
#else
         void ** ra = realloc( list->items, (100+list->maxCount)*sizeof( void * ));
         if( NULL == ra ) {
            status = UTIL_STATUS_TOO_MANY;
         }
         else {
            list->maxCount += 100;
            list->items     = ra;
         }
#endif
      }
      if( UTIL_STATUS_NO_ERROR == status ) {
         list->items[list->count++] = item;
      }
   }
   return status;
}
