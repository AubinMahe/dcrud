#pragma once
#include <dcrud/GUID.h>

typedef struct dcrudGUIDImpl_s {

   byte         platform;
   byte         exec;
   byte         cache;
   unsigned int instance;

} dcrudGUIDImpl;

dcrudGUIDImpl * dcrudGUID_init    ( void );
bool            dcrudGUID_isShared( const dcrudGUIDImpl * This );
void            dcrudGUID_set     ( dcrudGUID This, const dcrudGUID right );
