#include "Family.h"
#include <util/Pool.h>

#include <stdlib.h>
#include <string.h>
#include "poolSizes.h"

UTIL_POOL_DECLARE( channelTestFamily )

static int getClassID() {
   return 3;
}

static channelFactory TheFactory;

utilStatus channelTestFamily_registerFactory( void ) {
   TheFactory.getClassID = (channelGetClassID)getClassID;
   TheFactory.create     = (channelCreate    )channelTestFamily_new;
   TheFactory.encode     = (channelEncode    )channelTestFamily_encode;
   TheFactory.decode     = (channelDecode    )channelTestFamily_decode;
   return channelFactories_add( &TheFactory );
}

utilStatus channelTestFamily_getFactory( channelFactory ** target ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( target ) {
      *target = &TheFactory;
   }
   else {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   return status;
}

utilStatus channelTestFamily_new( channelTestFamily ** self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( self ) {
      channelTestFamily * This = *self;
      UTIL_ALLOCATE( channelTestFamily );
      if( UTIL_STATUS_NO_ERROR == status ) {
         status = channelTestFamily_init( This );
      }
   }
   else {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   return status;
}

utilStatus channelTestFamily_encode( const channelTestFamily * This, ioByteBuffer encoder ) {
   if( NULL == This ) {
      return UTIL_STATUS_NULL_ARGUMENT;
   }
   CHK(__FILE__,__LINE__,channelCoDec_putValue    ( encoder, &( This->parent1  )));
   CHK(__FILE__,__LINE__,channelCoDec_putReference( encoder,    This->parent2   ));
   CHK(__FILE__,__LINE__,channelCoDec_putList     ( encoder, &( This->children )));
   return UTIL_STATUS_NO_ERROR;
}

utilStatus channelTestFamily_decode( channelTestFamily * This, ioByteBuffer decoder ) {
   if( NULL == This ) {
      return UTIL_STATUS_NULL_ARGUMENT;
   }
   CHK(__FILE__,__LINE__,channelCoDec_getValue    ( decoder, &( This->parent1  )));
   CHK(__FILE__,__LINE__,channelCoDec_getReference( decoder, &( This->parent2  )));
   CHK(__FILE__,__LINE__,channelCoDec_getList     ( decoder, &( This->children )));
   return UTIL_STATUS_NO_ERROR;
}

utilStatus channelTestFamily_init( channelTestFamily * This ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( This ) {
      This->factory = &TheFactory;
      CHK(__FILE__,__LINE__,channelTestPerson_init( &( This->parent1 )));
      CHK(__FILE__,__LINE__,channelList_init((channelList *)&( This->children )));
   }
   else {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   return status;
}

utilStatus channelTestFamily_done( channelTestFamily * This ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( This ) {
      unsigned i;
      if( This->parent2 ) {
         CHK(__FILE__,__LINE__,channelTestPerson_delete( &This->parent2 ))
      }
      for( i = 0; i < This->children.count; ++i ) {
         CHK(__FILE__,__LINE__,channelTestPerson_delete( &This->children.items[i] ))
      }
      CHK(__FILE__,__LINE__,channelList_done((channelList *)&This->children ))
   }
   else {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   return status;
}

utilStatus channelTestFamily_toString(
   const channelTestFamily * This,
   char *                    target,
   size_t *                  size   )
{
   unsigned i;
   if( NULL == This ) {
      return UTIL_STATUS_NULL_ARGUMENT;
   }
   if( NULL == target ) {
      return UTIL_STATUS_NULL_ARGUMENT;
   }
   if( NULL == size ) {
      return UTIL_STATUS_NULL_ARGUMENT;
   }
   CHK(__FILE__,__LINE__,channelTestPerson_toString( &( This->parent1 ), target, size ));
   if( *size < 1 ) {
      return UTIL_STATUS_OVERFLOW;
   }
   strcat( target, "," );
   (*size) -= 1;
   CHK(__FILE__,__LINE__,channelTestPerson_toString(    This->parent2  , target, size ));
   if( *size < 2 ) {
      return UTIL_STATUS_OVERFLOW;
   }
   strcat( target, ",[" );
   (*size) -= 2;
   for( i = 0; i < This->children.count; ++i ) {
      CHK(__FILE__,__LINE__,channelTestPerson_toString( This->children.items[i], target, size ));
      if( i+1 < This->children.count ) {
         if( *size < 1 ) {
            return UTIL_STATUS_OVERFLOW;
         }
         strcat( target, "," );
         (*size) -= 1;
      }
   }
   if( *size < 1 ) {
      return UTIL_STATUS_OVERFLOW;
   }
   strcat( target, "]" );
   (*size) -= 1;
   return UTIL_STATUS_NO_ERROR;
}

utilStatus channelTestFamily_delete( channelTestFamily ** self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( NULL == self ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      channelTestFamily_done( *self );
      UTIL_RELEASE( channelTestFamily );
   }
   return status;
}
