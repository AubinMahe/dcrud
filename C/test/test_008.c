#include <dcrud/Network.h>
#include <os/System.h>
#include <util/CheckSysCall.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __linux__
#  include <sys/socket.h>
#  include <netdb.h>
#  include <time.h>
#  include <inttypes.h>
#elif _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <winsock2.h>
/*#  include <Ws2tcpip.h>*/
#  include <mswsock.h>
#endif

#define LOOP_COUNT 10000

/*
#define PRINT_SERIALIZE
#define PRINT_TIMING
*/

static dcrudLocalFactory rectangleFactory;
static dcrudLocalFactory ellipseFactory;

typedef struct FxColor_s {

   double red;
   double green;
   double blue;
   double opacity;

} FxColor;

static void FxColor_set( FxColor * This, const FxColor * source ) {
   This->red     = source->red;
   This->green   = source->green;
   This->blue    = source->blue;
   This->opacity = source->opacity;
}

typedef char ShareableShapeName[40];

typedef struct ShareableShape_s {

   ShareableShapeName name;
   double             x;
   double             y;
   double             w;
   double             h;
   FxColor            fill;
   FxColor            stroke;
   double             dx;
   double             dy;

} ShareableShape;

static void ShareableShape_set( ShareableShape * This, const ShareableShape * source ) {
   strncpy( This->name, source->name, sizeof( ShareableShapeName ));
   This->x = source->x;
   This->y = source->y;
   This->w = source->w;
   This->h = source->h;
   FxColor_set( &This->fill  , &source->fill   );
   FxColor_set( &This->stroke, &source->stroke );
}

static void ShareableShape_serialize( const ShareableShape * This, ioByteBuffer target ) {
   ioByteBuffer_putString( target, This->name );
   ioByteBuffer_putDouble( target, This->x );
   ioByteBuffer_putDouble( target, This->y );
   ioByteBuffer_putDouble( target, This->w );
   ioByteBuffer_putDouble( target, This->h );
   ioByteBuffer_putDouble( target, This->fill  .red );
   ioByteBuffer_putDouble( target, This->fill  .green );
   ioByteBuffer_putDouble( target, This->fill  .blue );
   ioByteBuffer_putDouble( target, This->fill  .opacity );
   ioByteBuffer_putDouble( target, This->stroke.red );
   ioByteBuffer_putDouble( target, This->stroke.green );
   ioByteBuffer_putDouble( target, This->stroke.blue );
   ioByteBuffer_putDouble( target, This->stroke.opacity );
#ifdef PRINT_SERIALIZE
   fprintf( stderr, "-- Serialize --\n" );
   fprintf( stderr, "name          : %s\n", This->name );
   fprintf( stderr, "x             : %f\n", This->x );
   fprintf( stderr, "y             : %f\n", This->y );
   fprintf( stderr, "w             : %f\n", This->w );
   fprintf( stderr, "h             : %f\n", This->h );
   fprintf( stderr, "fill  .red    : %f\n", This->fill  .red     );
   fprintf( stderr, "fill  .green  : %f\n", This->fill  .green   );
   fprintf( stderr, "fill  .blue   : %f\n", This->fill  .blue    );
   fprintf( stderr, "fill  .opacity: %f\n", This->fill  .opacity );
   fprintf( stderr, "stroke.red    : %f\n", This->stroke.red     );
   fprintf( stderr, "stroke.green  : %f\n", This->stroke.green   );
   fprintf( stderr, "stroke.blue   : %f\n", This->stroke.blue    );
   fprintf( stderr, "stroke.opacity: %f\n", This->stroke.opacity );
#endif
}

static void ShareableShape_unserialize( ShareableShape * This, ioByteBuffer source ) {
   ioByteBuffer_getString( source, This->name, sizeof( ShareableShapeName ));
   ioByteBuffer_getDouble( source, &This->x );
   ioByteBuffer_getDouble( source, &This->y );
   ioByteBuffer_getDouble( source, &This->w );
   ioByteBuffer_getDouble( source, &This->h );
   ioByteBuffer_getDouble( source, &This->fill  .red     );
   ioByteBuffer_getDouble( source, &This->fill  .green   );
   ioByteBuffer_getDouble( source, &This->fill  .blue    );
   ioByteBuffer_getDouble( source, &This->fill  .opacity );
   ioByteBuffer_getDouble( source, &This->stroke.red     );
   ioByteBuffer_getDouble( source, &This->stroke.green   );
   ioByteBuffer_getDouble( source, &This->stroke.blue    );
   ioByteBuffer_getDouble( source, &This->stroke.opacity );
}

static double nextDouble( double max, double min ) {
   return min + ((double)rand() / RAND_MAX )*( max - min );
}

static unsigned int g_rank;

static bool ShareableShape_init( dcrudShareable shareable ) {
   dcrudClassID     classID = dcrudShareable_getClassID( shareable );
   ShareableShape * shape   = (ShareableShape *)dcrudShareable_getUserData( shareable );

   if( 0 == dcrudClassID_compareTo( &classID, &rectangleFactory.classID )) {
      sprintf( shape->name, "Rectangle %03u", ++g_rank );
   }
   else if( 0 == dcrudClassID_compareTo( &classID, &ellipseFactory.classID )) {
      sprintf( shape->name, "Ellipse %03u", ++g_rank );
   }
   else {
      return false;
   }
   shape->x              = nextDouble( 540.0,  0.0 );
   shape->y              = nextDouble( 400.0,  0.0 );
   shape->w              = nextDouble( 100.0, 40.0 );
   shape->h              = nextDouble(  80.0, 20.0 );
   shape->fill.red       = nextDouble(   1.0,  0.0 );
   shape->fill.green     = nextDouble(   1.0,  0.0 );
   shape->fill.blue      = nextDouble(   1.0,  0.0 );
   shape->fill.opacity   = nextDouble(   1.0,  0.0 );
   shape->stroke.red     = nextDouble(   1.0,  0.0 );
   shape->stroke.green   = nextDouble(   1.0,  0.0 );
   shape->stroke.blue    = nextDouble(   1.0,  0.0 );
   shape->stroke.opacity = nextDouble(   1.0,  0.0 );
   shape->dx             = 1.0;
   shape->dy             = 1.0;
   return true;
}

static double areaMaxX = 640.0;
static double areaMaxY = 480.0;
#define MOVE 2.0

static bool moveShape( collForeach * context ) {
   collMapPair *    pair      = (collMapPair *   )context->item;
   dcrudICache      cache     = (dcrudICache     )context->user;
   dcrudShareable   shareable = (dcrudShareable  )pair->value;
   ShareableShape * shape     = (ShareableShape *)dcrudShareable_getUserData( shareable );
   bool             outOfBounds;

   do {
      outOfBounds = false;
      shape->x += nextDouble( 1.0, 0.0 )*shape->dx;
      shape->y += nextDouble( 1.0, 0.0 )*shape->dy;
      if( shape->x < 0 ) {
         outOfBounds = true;
         shape->dx = +MOVE;
      }
      else if( shape->x+shape->w > areaMaxX ) {
         outOfBounds = true;
         shape->dx = -MOVE;
      }
      if( shape->y < 0 ) {
         outOfBounds = true;
         shape->dy = +MOVE;
      }
      else if( shape->y+shape->h > areaMaxY ) {
         outOfBounds = true;
         shape->dy = -MOVE;
      }
   } while( outOfBounds );
   dcrudICache_update( cache, dcrudShareable_getShareable( shape ));
   return true;
}

static bool removeFromCache( collForeach * context ) {
   collMapPair *  pair      = (collMapPair *   )context->item;
   dcrudICache    cache     = (dcrudICache     )context->user;
   dcrudShareable shareable = (dcrudShareable  )pair->value;
   dcrudICache_delete( cache, shareable );
   return true;
}

static bool deleteShape( collForeach * context ) {
   collMapPair *  pair      = (collMapPair *   )context->item;
   dcrudShareable shareable = (dcrudShareable  )pair->value;
   dcrudShareable_delete( &shareable );
   return true;
}

static collMap * createShapes( dcrudIParticipant participant, collMap args ) {
   dcrudClassID     clazz     = collMap_get( args, "class" );
   double *         x         = collMap_get( args, "x" );
   double *         y         = collMap_get( args, "y" );
   double *         w         = collMap_get( args, "w" );
   double *         h         = collMap_get( args, "h" );
   dcrudShareable   shareable = dcrudIParticipant_createShareable( participant, clazz );
   ShareableShape * shape     = (ShareableShape *)dcrudShareable_getUserData( shareable );
   dcrudICache      cache    = dcrudIParticipant_getCache( participant, 0 );
   shape->x = *x;
   shape->y = *y;
   shape->w = *w;
   shape->h = *h;
   dcrudICache_create( cache, shareable );
   return 0;
}

void test_008( void ) {
   dcrudIParticipant participant = dcrudNetwork_join( 2, "224.0.0.3", 2417, "192.168.1.7" );
   if( participant ) {
      int              i;
#ifdef PRINT_TIMING
      uint64_t         prev          = osSystem_nanotime();
#endif
      dcrudICache      cache         = NULL;
      dcrudIDispatcher dispatcher    = NULL;
      dcrudIProvided   shapesFactory = NULL;

      rectangleFactory.classID     = dcrudClassID_new( 1, 1, 1, 1 );
      rectangleFactory.size        = sizeof( ShareableShape );
      rectangleFactory.initialize  = (dcrudLocalFactory_Initialize )ShareableShape_init;
      rectangleFactory.set         = (dcrudLocalFactory_Set        )ShareableShape_set;
      rectangleFactory.serialize   = (dcrudLocalFactory_Serialize  )ShareableShape_serialize;
      rectangleFactory.unserialize = (dcrudLocalFactory_Unserialize)ShareableShape_unserialize;
      ellipseFactory  .classID     = dcrudClassID_new( 1, 1, 1, 2 );
      ellipseFactory  .size        = sizeof( ShareableShape );
      ellipseFactory  .initialize  = (dcrudLocalFactory_Initialize )ShareableShape_init;
      ellipseFactory  .set         = (dcrudLocalFactory_Set        )ShareableShape_set;
      ellipseFactory  .serialize   = (dcrudLocalFactory_Serialize  )ShareableShape_serialize;
      ellipseFactory  .unserialize = (dcrudLocalFactory_Unserialize)ShareableShape_unserialize;
      dcrudIParticipant_registerLocalFactory( participant, &rectangleFactory );
      dcrudIParticipant_registerLocalFactory( participant, &ellipseFactory );
      dcrudIParticipant_createCache( participant, &cache );
      dcrudICache_create( cache, dcrudIParticipant_createShareable( participant, rectangleFactory.classID ));
      dcrudICache_create( cache, dcrudIParticipant_createShareable( participant, ellipseFactory  .classID ));
      dcrudICache_create( cache, dcrudIParticipant_createShareable( participant, rectangleFactory.classID ));
      dcrudICache_create( cache, dcrudIParticipant_createShareable( participant, ellipseFactory  .classID ));
      dispatcher    = dcrudIParticipant_getDispatcher( participant );
      shapesFactory = dcrudIDispatcher_provide( dispatcher, "IShapesFactory" );
      dcrudIProvided_addOperation(
         shapesFactory, "create", participant, (dcrudIOperation)createShapes );
      printf( "Publish every 40 ms, %d times.\n", LOOP_COUNT );
      for( i = 0; i < LOOP_COUNT; ++i ) {
#ifdef PRINT_TIMING
         uint64_t now = osSystem_nanotime();
         fprintf( stderr, "-- Publish, delta = %7.2f ------------------------------------------\n",
            ((double)(now-prev))/1000000.0 );
         prev = now;
#endif
         dcrudICache_publish( cache );
         osSystem_sleep( 40U );
         dcrudICache_foreach( cache, moveShape, cache );
         dcrudIDispatcher_handleRequests( dispatcher );
      }
      dcrudICache_foreach( cache, removeFromCache, cache );
      dcrudICache_publish( cache );
      dcrudICache_foreach( cache, deleteShape, NULL );
      dcrudClassID_delete( &rectangleFactory.classID );
      dcrudClassID_delete( &ellipseFactory  .classID );
      dcrudIParticipant_delete( &participant );
      printf( "Well done.\n" );
   }
   else {
      fprintf( stderr, "Unable to join network.\n" );
   }
}
