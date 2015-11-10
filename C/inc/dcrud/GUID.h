#pragma once
#include <util/types.h>

typedef struct dcrudGUID_s {

   int source;
   int instance;

} dcrudGUID;

void dcrudGUID_init       ( /* */ dcrudGUID * This, int source, int instance );
bool dcrudGUID_matchSource( const dcrudGUID * This, int source );
bool dcrudGUID_toString   ( const dcrudGUID * This, char * target, size_t targetSize );
int  dcrudGUID_compareTo  ( const dcrudGUID * * left, const dcrudGUID * * right );
