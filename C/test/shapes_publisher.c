#include <dcrud/Networks.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __linux__
#  include <sys/socket.h>
#  include <netdb.h>
#  include <time.h>
#elif _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <winsock2.h>
#  include <Ws2tcpip.h>
#  include <mswsock.h>
#endif

static const char *         MC_GROUP  = "224.0.0.3";
static const unsigned short MC_PORT   = 2416;
static const char *         MC_INTRFC = "192.168.1.6";

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
   fprintf( stderr, "-- Serialize --\n" );
   fprintf( stderr, "name          : %s\n", This->name           );
   fprintf( stderr, "x             : %f\n", This->x              );
   fprintf( stderr, "y             : %f\n", This->y              );
   fprintf( stderr, "w             : %f\n", This->w              );
   fprintf( stderr, "h             : %f\n", This->h              );
   fprintf( stderr, "fill  .red    : %f\n", This->fill  .red     );
   fprintf( stderr, "fill  .green  : %f\n", This->fill  .green   );
   fprintf( stderr, "fill  .blue   : %f\n", This->fill  .blue    );
   fprintf( stderr, "fill  .opacity: %f\n", This->fill  .opacity );
   fprintf( stderr, "stroke.red    : %f\n", This->stroke.red     );
   fprintf( stderr, "stroke.green  : %f\n", This->stroke.green   );
   fprintf( stderr, "stroke.blue   : %f\n", This->stroke.blue    );
   fprintf( stderr, "stroke.opacity: %f\n", This->stroke.opacity );
   fprintf( stderr, "----------------------------------------\n" );
}

static void ShareableShape_unserialize( ioByteBuffer source ) {
   (void)source;
}

static double nextDouble( double max, double min ) {
   return min + ((double)rand() / RAND_MAX )*( max - min );
}

static unsigned int g_rank;

static bool ShareableShape_init( dcrudShareable shareable ) {
   dcrudClassID     classID = dcrudShareable_getClassID( shareable );
   ShareableShape * shape   = (ShareableShape *)dcrudShareable_getUserData( shareable );

   if( classID == rectangleClassID ) {
      sprintf( shape->name, "%s %03u", "Rectangle", ++g_rank );
   }
   else if( classID == ellipseClassID ) {
      sprintf( shape->name, "%s %03u", "Ellipse", ++g_rank );
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
   dcrudICache_update( shapes, shape->base );
}

int shapesPublisherTests( int argc, char * argv[] ) {
   const char *   address    = MC_GROUP;
   unsigned short port       = MC_PORT;
   const char *   intrfc     = MC_INTRFC;
   byte           platformId = 0;
   byte           execId     = 0;
   int               i;
   dcrudIParticipant participant;
   for( i = 1; i < argc; ++i ) {
      if( 0 == strcmp( "--address", argv[i] )) {
         address = argv[++i];
      }
      else if( 0 == strcmp( "--port", argv[i] )) {
         port = (unsigned short)atoi( argv[++i] );
      }
      else if( 0 == strcmp( "--interface", argv[i] )) {
         intrfc = argv[++i];
      }
      else if( 0 == strcmp( "--platform-id", argv[i] )) {
         platformId = (byte)atoi( argv[++i] );
      }
      else if( 0 == strcmp( "--exec-id", argv[i] )) {
         execId     = (byte)atoi( argv[++i] );
      }
      else {
         fprintf( stderr, "unexpected argument: %s\n", argv[i] );
         exit(-1);
      }
   }
   if( platformId < 1 ) {
      fprintf( stderr, "--platfom-id <id> must be >= 0\n" );
      exit(-1);
   }
   if( execId < 1 ) {
      fprintf( stderr, "--exec-id <id> must be >= 0\n" );
      exit(-1);
   }
   participant = dcrudNetworks_join( address, intrfc, port, platformId, execId );
   if( participant ) {
      dcrudICache    shapes;
      dcrudShareable rect1;
      dcrudShareable elli1;
      dcrudShareable rect2;
      dcrudShareable elli2;

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
      dcrudICache_create( shapes, rect1 );
      dcrudICache_create( shapes, elli1 );
      dcrudICache_create( shapes, rect2 );
      dcrudICache_create( shapes, elli2 );
      for( i = 0; i < 1000; ++i ) {
         struct timespec req = { 0, 40*1000*1000 };
         dcrudICache_publish( shapes );
         nanosleep( &req, NULL );
         move( shapes, rect1 );
         move( shapes, elli1 );
         move( shapes, rect2 );
         move( shapes, elli2 );
      }
      printf( "Well done.\n" );
   }
   else {
      fprintf( stderr, "Error catched." );
   }
   return 0;
}
