#pragma once
#include <dcrud/GUID.h>

typedef struct dcrudGUIDImpl_s {

   unsigned short publisher;
   byte           cache;
   unsigned int   instance;

} dcrudGUIDImpl;

dcrudGUID dcrudGUID_new     ( void );
void      dcrudGUID_delete  ( dcrudGUID * This );
bool      dcrudGUID_isShared( const dcrudGUID This );
void      dcrudGUID_set     ( dcrudGUID This, const dcrudGUID right );
void      dcrudGUID_init    ( dcrudGUID This, unsigned short publisherId, byte cacheId, unsigned int nextInstance );
