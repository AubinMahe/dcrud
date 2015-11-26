#include <dcrud/IParticipant.h>

typedef struct dcrudShareableImpl_s {

   dcrudGUID                  id;
   dcrudClassID               classID;
   dcrudShareable_Serialize   serialize;
   dcrudShareable_Unserialize unserialize;

} dcrudShareableImpl;
