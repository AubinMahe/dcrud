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

static const byte RECTANGLE_CLASS_ID = 1;
static const byte ELLIPSE_CLASS_ID   = 2;

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

static ShareableShape * createShapeOfClass( byte classId ) {
   ShareableShape * shape = (ShareableShape *)malloc( sizeof( ShareableShape ));
   memset( shape, 0, sizeof( ShareableShape ));
   dcrudShareable_init(
      shape,
      &shape->base,
      (dcrudShareable_setF        )ShareableShape_set,
      (dcrudShareable_serializeF  )ShareableShape_serialize,
      (dcrudShareable_unserializeF)ShareableShape_unserialize );
   sprintf( shape->name, "%s %03u",
      ( classId == RECTANGLE_CLASS_ID ) ? "Rectangle" : "Ellipse",
      ++g_rank );
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
   return shape;
}

static dcrudShareable rectangleFactory() {
   return createShapeOfClass( RECTANGLE_CLASS_ID )->base;
}

static dcrudShareable ellipseFactory() {
   return createShapeOfClass( ELLIPSE_CLASS_ID )->base;
}

static double areaMaxX = 640.0;
static double areaMaxY = 480.0;
#define MOVE 2.0

static void move( dcrudICache shapes, ShareableShape * shape ) {
   bool outOfBounds;
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
      dcrudClassID     rectangleClass = dcrudClassID_init( 1, 1, 1, RECTANGLE_CLASS_ID );
      dcrudClassID     ellipseClass   = dcrudClassID_init( 1, 1, 1, ELLIPSE_CLASS_ID );
      dcrudICache      shapes;
      ShareableShape * rect1;
      ShareableShape * elli1;
      ShareableShape * rect2;
      ShareableShape * elli2;
      dcrudIParticipant_registerClass( participant, rectangleClass, rectangleFactory );
      dcrudIParticipant_registerClass( participant, ellipseClass  , ellipseFactory );
      dcrudIParticipant_createCache( participant, &shapes );
      rect1  = createShapeOfClass( RECTANGLE_CLASS_ID );
      elli1  = createShapeOfClass( ELLIPSE_CLASS_ID );
      rect2  = createShapeOfClass( RECTANGLE_CLASS_ID );
      elli2  = createShapeOfClass( ELLIPSE_CLASS_ID );
      dcrudICache_create( shapes, rect1->base );
      dcrudICache_create( shapes, elli1->base );
      dcrudICache_create( shapes, rect2->base );
      dcrudICache_create( shapes, elli2->base );
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
