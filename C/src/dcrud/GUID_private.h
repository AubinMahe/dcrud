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

utilStatus dcrudGUID_new      ( /* */ dcrudGUID * This );
utilStatus dcrudGUID_clone    ( /* */ dcrudGUID * This, const dcrudGUID ref );
utilStatus dcrudGUID_isShared ( const dcrudGUID   This, bool * shared );
utilStatus dcrudGUID_isOwnedBy( const dcrudGUID   This, unsigned publisherId, bool * isOwnedBy );
utilStatus dcrudGUID_set      ( /* */ dcrudGUID   This, const dcrudGUID right );
utilStatus dcrudGUID_init     ( /* */ dcrudGUID   This, unsigned int publisherId, unsigned int nextInstance );

#ifdef __cplusplus
}
#endif
