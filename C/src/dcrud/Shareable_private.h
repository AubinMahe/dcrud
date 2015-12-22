#include <dcrud/IParticipant.h>

typedef struct dcrudShareableImpl_s {

   dcrudGUID                     id;
   dcrudClassID                  classID;
   dcrudLocalFactory_Serialize   serialize;
   dcrudLocalFactory_Unserialize unserialize;

} dcrudShareableImpl;

dcrudShareable dcrudShareable_new( dcrudLocalFactory * meta, dcrudClassID classID );
