#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <dcrud/GUID.h>

typedef struct dcrudGUIDImpl_s {

   unsigned int magic;
   unsigned int publisher;
   unsigned int instance;

} dcrudGUIDImpl;

utilStatus dcrudGUID_new      ( dcrudGUID * target );
utilStatus dcrudGUID_delete   ( dcrudGUID * This );
utilStatus dcrudGUID_isShared ( const dcrudGUID self, bool * shared );
utilStatus dcrudGUID_isOwnedBy( const dcrudGUID self, unsigned publisherId, bool * isOwnedBy );
utilStatus dcrudGUID_set      ( dcrudGUID This, const dcrudGUID right );
utilStatus dcrudGUID_init     ( dcrudGUID This, unsigned int publisherId, unsigned int nextInstance );

#ifdef __cplusplus
}
#endif
