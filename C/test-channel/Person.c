#include "Person.h"
#include <util/Pool.h>
#include <stdlib.h>
#include <string.h>

#include "../inc/util/String.h"
#include "poolSizes.h"

UTIL_POOL_DECLARE( channelTestPerson )

static int getClassID() {
   return 2;
}

static channelFactory TheFactory;

utilStatus channelTestPerson_registerFactory() {
   TheFactory.getClassID = (channelGetClassID)getClassID;
   TheFactory.create     = (channelCreate    )channelTestPerson_new;
   TheFactory.encode     = (channelEncode    )channelTestPerson_encode;
   TheFactory.decode     = (channelDecode    )channelTestPerson_decode;
   CHK(__FILE__,__LINE__,channelFactories_add( &TheFactory ));
   return UTIL_STATUS_NO_ERROR;
}

utilStatus channelTestPerson_getFactory( channelFactory ** target ) {
   if( target ) {
      *target = &TheFactory;
      return UTIL_STATUS_NO_ERROR;
   }
   return UTIL_STATUS_NULL_ARGUMENT;
}

utilStatus channelTestPerson_new( channelTestPerson ** self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( self ) {
      channelTestPerson * This = *self;
      UTIL_ALLOCATE( channelTestPerson );
      if( UTIL_STATUS_NO_ERROR == status ) {
         status = channelTestPerson_init( This );
      }
   }
   else {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   return status;
}

utilStatus channelTestPerson_delete( channelTestPerson ** self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   UTIL_RELEASE( channelTestPerson )
   return status;
}

utilStatus channelTestPerson_encode( const channelTestPerson * This, ioByteBuffer encoder ) {
   if( NULL == This ) {
      return UTIL_STATUS_NULL_ARGUMENT;
   }
   CHK(__FILE__,__LINE__,ioByteBuffer_putString( encoder, This->forname ));
   CHK(__FILE__,__LINE__,ioByteBuffer_putString( encoder, This->name ));
   CHK(__FILE__,__LINE__,channelCoDec_putValue ( encoder, &( This->birthdate )));
   return UTIL_STATUS_NO_ERROR;
}

utilStatus channelTestPerson_decode( channelTestPerson * This, ioByteBuffer decoder ) {
   if( NULL == This ) {
      return UTIL_STATUS_NULL_ARGUMENT;
   }
   CHK(__FILE__,__LINE__,ioByteBuffer_getString( decoder, (char *)This->forname, channelTestFORNAME_SIZE ));
   CHK(__FILE__,__LINE__,ioByteBuffer_getString( decoder, (char *)This->name   , channelTestNAME_SIZE ));
   CHK(__FILE__,__LINE__,channelCoDec_getValue ( decoder, &(      This->birthdate )));
   return UTIL_STATUS_NO_ERROR;
}

utilStatus channelTestPerson_init( channelTestPerson * target ) {
   if( target ) {
      target->factory   = &TheFactory;
      return channelTestISO8601Date_init( &( target->birthdate ));
   }
   return UTIL_STATUS_NULL_ARGUMENT;
}

utilStatus channelTestPerson_toString(
   const channelTestPerson * This,
   char *                    target,
   size_t *                  size   )
{
   if( NULL == This ) {
      return UTIL_STATUS_NULL_ARGUMENT;
   }
   if( NULL == target ) {
      return UTIL_STATUS_NULL_ARGUMENT;
   }
   if( NULL == size ) {
      return UTIL_STATUS_NULL_ARGUMENT;
   }
   if( *size < 1 ) {
      return UTIL_STATUS_OVERFLOW;
   }
   strcat( target, "{" );
   (*size) -= 1;
   CHK(__FILE__,__LINE__,utilString_concat( target, This->forname, size ));
   if( *size < 1 ) {
      return UTIL_STATUS_OVERFLOW;
   }
   strcat( target, "," );
   (*size) -= 1;
   CHK(__FILE__,__LINE__,utilString_concat( target, This->name   , size ));
   if( *size < 1 ) {
      return UTIL_STATUS_OVERFLOW;
   }
   strcat( target, "," );
   (*size) -= 1;
   CHK(__FILE__,__LINE__,channelTestISO8601Date_toString( &( This->birthdate ), target, size ));
   if( *size < 1 ) {
      return UTIL_STATUS_OVERFLOW;
   }
   strcat( target, "}" );
   (*size) -= 1;
   return UTIL_STATUS_NO_ERROR;
}
