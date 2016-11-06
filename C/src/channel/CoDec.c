#include <channel/CoDec.h>
#include <channel/Factories.h>
#include <channel/List.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

utilStatus channelCoDec_init( ioByteBuffer encoder, short id ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( NULL == encoder ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      status = ioByteBuffer_clear( encoder );
      if( status == UTIL_STATUS_NO_ERROR ) {
         status = ioByteBuffer_putShort( encoder, id );
      }
   }
   return status;
}

utilStatus channelCoDec_putReference( ioByteBuffer encoder, const void * ref ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
	if( NULL == encoder || NULL == ref ) {
	   status = UTIL_STATUS_NULL_ARGUMENT;
	}
	else {
	   channelFactory * factory = *(channelFactory **)ref;
	   int classID = factory->getClassID();
	   status = ioByteBuffer_putInt( encoder, classID );
	   if( UTIL_STATUS_NO_ERROR == status ) {
	      status = factory->encode( ref, encoder );
	   }
	}
	return status;
}

utilStatus channelCoDec_getReference( ioByteBuffer decoder, void * target ) {
	utilStatus status = UTIL_STATUS_NO_ERROR;
	if( NULL == decoder || NULL == target ) {
	   status = UTIL_STATUS_NULL_ARGUMENT;
	}
	else {
	   int classID = 0;
      status = ioByteBuffer_getInt( decoder, &classID );
      if( UTIL_STATUS_NO_ERROR == status ) {
         channelFactory * factory = NULL;
         status = channelFactories_get( classID, &factory );
         if( UTIL_STATUS_NO_ERROR == status ) {
            void ** targetPtr = (void **)target;
            status = factory->create( targetPtr );
            if( UTIL_STATUS_NO_ERROR == status ) {
               status = factory->decode( *targetPtr, decoder );
            }
         }
      }
	}
	return status;
}

utilStatus channelCoDec_putValue( ioByteBuffer encoder, const void * source ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( NULL == encoder || NULL == source ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      channelFactory * factory = *(channelFactory **)source;
      status = factory->encode( source, encoder );
   }
   return status;
}

utilStatus channelCoDec_getValue( ioByteBuffer decoder, void * target ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( NULL == decoder || NULL == target ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      channelFactory * factory = *(channelFactory **)target;
      status = factory->decode( target, decoder );
   }
   return status;
}

utilStatus channelCoDec_putList( ioByteBuffer encoder, const void * source ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( NULL == encoder || NULL == source ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      unsigned i;
      const channelList * lst = (const channelList *)source;
      status = ioByteBuffer_putUInt( encoder, lst->count );
      for( i = 0U; ( UTIL_STATUS_NO_ERROR == status ) && ( i < lst->count ); ++i ) {
         const void * ref = lst->items[i];
         if( ! ref ) {
            status = ioByteBuffer_putInt( encoder, 0 );
         }
         else {
            channelFactory * factory = *(channelFactory **)ref;
            int classID = factory->getClassID();
            status = ioByteBuffer_putInt( encoder, classID );
            if( UTIL_STATUS_NO_ERROR == status ) {
               status = factory->encode( ref, encoder );
            }
         }
      }
   }
	return status;
}

utilStatus channelCoDec_getList( ioByteBuffer decoder, void * target ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( NULL == decoder || NULL == target ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      channelList * lst = (channelList *)target;
      unsigned i, count;
      status = ioByteBuffer_getInt( decoder, (int *)&count );
      for( i = 0U; ( UTIL_STATUS_NO_ERROR == status ) && ( i < count) ; ++i ) {
         void * item = NULL;
         status = channelCoDec_getReference( decoder, &item );
         if( UTIL_STATUS_NO_ERROR == status ) {
            status = channelList_addItem( lst, item );
         }
      }
   }
	return status;
}
