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
#  include <Ws2tcpip.h>
#  include <mswsock.h>
#endif
/*
#define PRINT_SERIALIZE
#define PRINT_TIMING
*/
dcrudClassID rectangleClassID;
dcrudClassID ellipseClassID;

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

   if( classID == rectangleClassID ) {
      sprintf( shape->name, "Rectangle %03u", ++g_rank );
   }
   else if( classID == ellipseClassID ) {
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

static void move( dcrudICache shapes, dcrudShareable shareable ) {
   bool             outOfBounds;
   ShareableShape * shape = (ShareableShape *)dcrudShareable_getUserData( shareable );
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
   dcrudICache_update( shapes, dcrudShareable_getShareable( shape ));
}

static void createShapes( dcrudICache shapes, collMap in, collMap out ) {
   printf( "createShapes remotely called!\n" );
   (void)shapes;
   (void)in;
   (void)out;
}

int shapesPublisherTests( int argc, char * argv[] ) {
   const char *      pubName     = NULL;
   const char *      intrfc      = NULL;
   dcrudIParticipant participant = NULL;
   int               i;

   for( i = 2; i < argc; ++i ) {
      if( 0 == strcmp( argv[i], "--pub-name" )) {
         pubName = argv[++i];
      }
      else if( 0 == strcmp( argv[i], "--interface" )) {
         intrfc = argv[++i];
      }
   }
   if( !pubName ) {
      fprintf( stderr, "%s --pub-name <publisher-name> is mandatory\n", argv[0] );
      exit(-1);
   }
   if( !intrfc ) {
      fprintf( stderr, "%s --interface <ipv4> is mandatory\n", argv[0] );
      exit(-1);
   }
   participant = dcrudNetwork_join( "network.xml", intrfc, pubName );
   if( participant ) {
#ifdef PRINT_TIMING
      static uint64_t prev = osSystem_nanotime();
#endif
      dcrudICache      shapes        = NULL;
      dcrudShareable   rect1         = NULL;
      dcrudShareable   elli1         = NULL;
      dcrudShareable   rect2         = NULL;
      dcrudShareable   elli2         = NULL;
      dcrudIDispatcher dispatcher    = NULL;
      dcrudIProvided   shapesFactory = NULL;

      rectangleClassID = dcrudClassID_new( 1, 1, 1, 1 );
      ellipseClassID   = dcrudClassID_new( 1, 1, 1, 2 );
      dcrudIParticipant_registerClass(
         participant,
         rectangleClassID,
         sizeof( ShareableShape ),
         (dcrudShareable_Initialize )ShareableShape_init,
         (dcrudShareable_Set        )ShareableShape_set,
         (dcrudShareable_Serialize  )ShareableShape_serialize,
         (dcrudShareable_Unserialize)ShareableShape_unserialize );
      dcrudIParticipant_registerClass(
         participant,
         ellipseClassID,
         sizeof( ShareableShape ),
         (dcrudShareable_Initialize )ShareableShape_init,
         (dcrudShareable_Set        )ShareableShape_set,
         (dcrudShareable_Serialize  )ShareableShape_serialize,
         (dcrudShareable_Unserialize)ShareableShape_unserialize );
      dcrudIParticipant_createCache( participant, &shapes );
      rect1 = dcrudIParticipant_createShareable( participant, rectangleClassID );
      elli1 = dcrudIParticipant_createShareable( participant, ellipseClassID );
      rect2 = dcrudIParticipant_createShareable( participant, rectangleClassID );
      elli2 = dcrudIParticipant_createShareable( participant, ellipseClassID );
      dispatcher    = dcrudIParticipant_getDispatcher( participant );
      shapesFactory = dcrudIDispatcher_provide( dispatcher, "IShapesFactory" );
      dcrudIProvided_addOperation( shapesFactory, "create", shapes, (dcrudIOperation)createShapes );
      dcrudICache_create( shapes, rect1 );
      dcrudICache_create( shapes, elli1 );
      dcrudICache_create( shapes, rect2 );
      dcrudICache_create( shapes, elli2 );
      printf( "Publishing 4 shapes every 40 ms, 10 000 times.\n" );
      for( i = 0; i < 10000; ++i ) {
#ifdef PRINT_TIMING
         uint64_t now = osSystem_nanotime();
         fprintf( stderr, "-- Publish, delta = %7.2f ------------------------------------------\n",
            ((double)(now-prev))/1000000.0 );
         prev = now;
#endif
         dcrudICache_publish( shapes );
         osSystem_sleep( 40U );
         move( shapes, rect1 );
         move( shapes, elli1 );
         move( shapes, rect2 );
         move( shapes, elli2 );
      }
      dcrudShareable_delete( &rect1 );
      dcrudShareable_delete( &elli1 );
      dcrudShareable_delete( &rect2 );
      dcrudShareable_delete( &elli2 );
      dcrudClassID_delete( &rectangleClassID );
      dcrudClassID_delete( &ellipseClassID );
      printf( "Well done.\n" );
      dcrudNetwork_leave( &participant );
   }
   else {
      fprintf( stderr, "Unable to join network.\n" );
   }
   return 0;
}
