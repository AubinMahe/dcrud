#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <util/types.h>

typedef int( * collComparator)( const void * left, const void * right );

int collByteCompare   ( const byte **           left, const byte **           right );
int collShortCompare  ( const short **          left, const short **          right );
int collUshortCompare ( const unsigned short ** left, const unsigned short ** right );
int collIntCompare    ( const int **            left, const int **            right );
int collUintCompare   ( const unsigned int **   left, const unsigned int **   right );
int collLongCompare   ( const int64_t **        left, const int64_t **        right );
int collUlongCompare  ( const uint64_t **       left, const uint64_t **       right );
int collFloatCompare  ( const float **          left, const float **          right );
int collDoubleCompare ( const double **         left, const double **         right );
int collStringCompare ( const char **           left, const char **           right );
int collPointerCompare( const void **           left, const void **           right );

#ifdef __cplusplus
}
#endif
