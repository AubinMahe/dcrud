#pragma once
#include <io/ByteBuffer.h>
#include <dcrud/GUID.h>

typedef void (* dcrudShareable_set        )( void * This, const void * source );
typedef void (* dcrudShareable_serialize  )( void * This, ioByteBuffer * target  );
typedef void (* dcrudShareable_unserialize)( void * This, ioByteBuffer * source  );

typedef struct dcrudShareable_s {

   int                        classId;
   dcrudGUID                  id;
   dcrudShareable_set         set;
   dcrudShareable_serialize   serialize;
   dcrudShareable_unserialize unserialize;

} dcrudShareable;
