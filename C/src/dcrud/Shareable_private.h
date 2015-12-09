#include <dcrud/IParticipant.h>

typedef struct dcrudShareableImpl_s {

   dcrudGUID                  id;
   dcrudClassID               classID;
   dcrudShareable_Serialize   serialize;
   dcrudShareable_Unserialize unserialize;

} dcrudShareableImpl;

typedef struct dcrudShareableClass_s {

   size_t                     size;
   dcrudShareable_Initialize  initialize;
   dcrudShareable_Set         set;
   dcrudShareable_Serialize   serialize;
   dcrudShareable_Unserialize unserialize;

} dcrudShareableClass;

dcrudShareable dcrudShareable_new( dcrudShareableClass * meta, dcrudClassID classID );
