#include <dcrud/ICallback.h>

typedef struct dcrudICallbackImpl_s {

   dcrudICallback_function callback;
   void *                  userData;

} dcrudICallbackImpl;

dcrudICallback dcrudICallback_new( dcrudICallback_function callback, void * userData ) {
   dcrudICallbackImpl * This = (dcrudICallbackImpl *)malloc( sizeof( dcrudICallbackImpl ));
   This->callback = callback;
   This->userData = userData;
   return (dcrudICallback)This;
}

void dcrudICallback_delete( dcrudICallback * This ) {
   free( *This );
   *This = NULL;
}

void dcrudICallback_callback(
   dcrudICallback self,
   const char *   intrfc,
   const char *   operation,
   collMap        results   )
{
   dcrudICallbackImpl * This = (dcrudICallbackImpl *)self;
   This->callback( self, intrfc, operation, results );
}

void * dcrudICallback_getUserData( dcrudICallback self ) {
   dcrudICallbackImpl * This = (dcrudICallbackImpl *)self;
   return This->userData;
}
