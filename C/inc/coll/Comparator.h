#pragma once

typedef int( * collComparator)( const void * left, const void * right );

int collStringComparator( const char * * left, const char * * right );
