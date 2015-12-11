#pragma once
#ifdef __cplusplus
extern "C" {
#endif

typedef int( * collComparator)( const void * left, const void * right );

int collStringComparator( const char * * left, const char * * right );

#ifdef __cplusplus
}
#endif
