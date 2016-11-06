#include <coll/Comparator.h>
#include <string.h>

#define NUMBER_CMP( N, T )\
int coll ## N ## Compare( const T ** left, const T ** right ) {\
   return (int)( *left - *right );\
}

NUMBER_CMP( Byte  , byte           )
NUMBER_CMP( Short , short          )
NUMBER_CMP( Ushort, unsigned short )
NUMBER_CMP( Int   , int            )
NUMBER_CMP( Uint  , unsigned int   )
NUMBER_CMP( Long  , int64_t        )
NUMBER_CMP( Ulong , uint64_t       )
NUMBER_CMP( Float , float          )
NUMBER_CMP( Double, double         )

int collStringCompare( const char ** left, const char ** right ) {
   return strcmp( *left, *right );
}

int collPointerCompare( const void ** left, const void ** right ) {
   return (uint64_t)*left - (uint64_t)*right;
}
