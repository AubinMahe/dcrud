#include <dcrud/Network.h>
#include <dcrud/DebugSettings.h>

#include <os/System.h>

#include <util/CmdLine.h>

#include <coll/Map.h>

#include <io/NetworkInterfaces.h>

#include "Settings.h"
#include "StaticRegistry.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static dcrudLocalFactory rectangleFactory;
static dcrudLocalFactory ellipseFactory;

typedef struct FxColor_s {

   double red;
   double green;
   double blue;
   double opacity;

} FxColor;

typedef char ShareableShapeName[40];

typedef struct ShareableShape_s {

   dcrudShareable     base;
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

/*
static void FxColor_set( FxColor * This, const FxColor * source ) {
   This->red     = source->red;
   This->green   = source->green;
   This->blue    = source->blue;
   This->opacity = source->opacity;
}

static unsigned int g_rank;

static utilStatus ShareableShape_init( dcrudShareable shareable ) {
   dcrudClassID       classID;
   dcrudShareableData data  = NULL;
   ShareableShape *   shape = NULL;
   utilStatus         status;

   status = dcrudShareable_getClassID( shareable, &classID );
   status = dcrudShareable_getUserData( shareable, &data );
   shape = (ShareableShape *)data;
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
   return status;
}
*/
static double areaMaxX = 640.0;
static double areaMaxY = 480.0;
#define MOVE 2.0

static utilStatus moveShape( collForeach * context ) {
   dcrudICache      cache     = (dcrudICache   )context->user;
   dcrudShareable   shareable = (dcrudShareable)context->value;
   ShareableShape * shape;
   bool             outOfBounds;
   utilStatus       status =
      dcrudShareable_getData( shareable, (dcrudShareableData *)&shape );
   if( UTIL_STATUS_NO_ERROR == status ) {
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
      status = dcrudICache_update( cache, shareable );
   }
   return status;
}

static utilStatus removeFromCache( collForeach * context ) {
   dcrudICache    cache     = (dcrudICache   )context->user;
   dcrudShareable shareable = (dcrudShareable)context->value;
   return dcrudICache_delete( cache, shareable );
}

static utilStatus deleteShape( collForeach * context ) {
   dcrudShareable shareable = (dcrudShareable)context->value;
   return dcrudShareable_delete( &shareable );
}

static dcrudArguments createShapes( void * param, dcrudArguments args ) {
   dcrudIParticipant participant = param;
   ShareableShape * shape = (ShareableShape *)malloc( sizeof( ShareableShape ));
   dcrudClassID     clazz;
   dcrudShareable   shareable;
   dcrudICache      cache;
   utilStatus       status;

   status = dcrudArguments_getClassID( args, "class", &clazz );
   if( UTIL_STATUS_NO_ERROR != status ) return NULL;
   status = dcrudArguments_getDouble( args, "x"    , &shape->x );
   if( UTIL_STATUS_NO_ERROR != status ) return NULL;
   status = dcrudArguments_getDouble( args, "y"    , &shape->y );
   if( UTIL_STATUS_NO_ERROR != status ) return NULL;
   status = dcrudArguments_getDouble( args, "w"    , &shape->w );
   if( UTIL_STATUS_NO_ERROR != status ) return NULL;
   status = dcrudArguments_getDouble( args, "h"    , &shape->h );
   if( UTIL_STATUS_NO_ERROR != status ) return NULL;
   status = dcrudIParticipant_createShareable( participant, clazz, shape, &shareable );
   if( UTIL_STATUS_NO_ERROR != status ) return NULL;
   status = dcrudIParticipant_getCache( participant, 0, &cache );
   if( UTIL_STATUS_NO_ERROR != status ) return NULL;
   dcrudICache_create( cache, shareable );
   return NULL;
}

static dcrudArguments exitSrvc( dcrudIParticipant participant, dcrudArguments args ) {
   dcrudICache cache = NULL;
   printf( "Press <enter> to exit\n" );
   fgetc( stdin );
   dcrudIParticipant_getDefaultCache( participant, &cache );
   dcrudICache_foreach( cache, removeFromCache, cache );
   dcrudICache_publish( cache );
   dcrudICache_foreach( cache, deleteShape, NULL );
   dcrudClassID_delete( &rectangleFactory.classID );
   dcrudClassID_delete( &ellipseFactory  .classID );
   dcrudNetwork_leave();
   printf( "Well done.\n" );
   exit(0);
   return NULL;
   (void)args;
}

utilStatus c_publisher_java_subscriber_Shapes( const utilCmdLine cmdLine ) {
   ioInetSocketAddress p1;
   dcrudIParticipant   participant;
   utilStatus          status;
   dcrudICache         cache         = NULL;
   dcrudIDispatcher    dispatcher    = NULL;
   dcrudIProvided      shapesFactory = NULL;
   dcrudIProvided      iMonitor      = NULL;
   dcrudShareable      rectangle;
   dcrudShareable      ellipse;
   bool                dumpReceivedBuffer;
   dcrudIRegistry      registry = NULL;
   dcrudShareableData  rectData = NULL;
   dcrudShareableData  ellipseData = NULL;

   CHK(__FILE__,__LINE__,utilCmdLine_getBoolean( cmdLine, "dump-received-buffer", &dumpReceivedBuffer ));
   dcrudDebugSettings->dumpNetworkReceiverOperations = dumpReceivedBuffer;

   CHK(__FILE__,__LINE__,getStaticRegistry( &registry ));
   CHK(__FILE__,__LINE__,ioInetSocketAddress_init( &p1, MCAST_ADDRESS, 2417 ));
   CHK(__FILE__,__LINE__,dcrudNetwork_join( 2, &p1, NETWORK_INTERFACE, &participant ));
   CHK(__FILE__,__LINE__,dcrudClassID_resolve( &rectangleFactory.classID, 1, 1, 1, 1 ));
   rectangleFactory.serialize   = (dcrudLocalFactory_Serialize  )ShareableShape_serialize;
   rectangleFactory.unserialize = (dcrudLocalFactory_Unserialize)ShareableShape_unserialize;
   CHK(__FILE__,__LINE__,dcrudClassID_resolve( &ellipseFactory.classID, 1, 1, 1, 2 ));
   ellipseFactory  .serialize   = (dcrudLocalFactory_Serialize  )ShareableShape_serialize;
   ellipseFactory  .unserialize = (dcrudLocalFactory_Unserialize)ShareableShape_unserialize;
   CHK(__FILE__,__LINE__,dcrudIParticipant_registerLocalFactory( participant, &rectangleFactory ));
   CHK(__FILE__,__LINE__,dcrudIParticipant_registerLocalFactory( participant, &ellipseFactory ));
   CHK(__FILE__,__LINE__,dcrudIParticipant_getDefaultCache( participant, &cache ));
   CHK(__FILE__,__LINE__,dcrudIParticipant_createShareable( participant, rectangleFactory.classID, rectData, &rectangle ));
   CHK(__FILE__,__LINE__,dcrudICache_create( cache, rectangle ));
   CHK(__FILE__,__LINE__,dcrudIParticipant_createShareable( participant, ellipseFactory  .classID, ellipseData, &ellipse ));
   CHK(__FILE__,__LINE__,dcrudICache_create( cache, ellipse ));
   CHK(__FILE__,__LINE__,dcrudIParticipant_createShareable( participant, rectangleFactory.classID, rectData, &rectangle ));
   CHK(__FILE__,__LINE__,dcrudICache_create( cache, rectangle ));
   CHK(__FILE__,__LINE__,dcrudIParticipant_createShareable( participant, ellipseFactory  .classID, ellipseData, &ellipse ));
   CHK(__FILE__,__LINE__,dcrudICache_create( cache, ellipse ));
   CHK(__FILE__,__LINE__,dcrudIParticipant_getDispatcher( participant, &dispatcher ));
   CHK(__FILE__,__LINE__,dcrudIDispatcher_provide( dispatcher, "IShapesFactory", &shapesFactory ));
   CHK(__FILE__,__LINE__,dcrudIDispatcher_provide( dispatcher, "IMonitor"      , &iMonitor ));
   CHK(__FILE__,__LINE__,dcrudIProvided_addOperation( shapesFactory, "create", participant, createShapes ));
   CHK(__FILE__,__LINE__,dcrudIProvided_addOperation(
      iMonitor     , "exit"  , participant, (dcrudIOperation)exitSrvc ));
   CHK(__FILE__,__LINE__,dcrudIParticipant_listen( participant, registry, NETWORK_INTERFACE ));
   printf( "Publish every 40 ms.\n" );
   for(;;) {
      CHK(__FILE__,__LINE__,dcrudICache_publish( cache ));
      CHK(__FILE__,__LINE__,osSystem_sleep( 40U ));
      CHK(__FILE__,__LINE__,dcrudICache_foreach( cache, moveShape, cache ));
      CHK(__FILE__,__LINE__,dcrudIDispatcher_handleRequests( dispatcher ));
   }
   return status;
}
