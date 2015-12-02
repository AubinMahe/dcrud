#include <coll/Comparator.h>
#include <string.h>

int collStringComparator( const char * * left, const char * * right ) {
   return strcmp( *left, *right );
}
