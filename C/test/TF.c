#include <dcrud/RepositoryFactoryBuilder.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef LINUX
#  include <sys/socket.h>
#  include <netdb.h>
#elif _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <winsock2.h>
#  include <Ws2tcpip.h>
#  include <mswsock.h>
#endif

static const char * MC_GROUP      = "224.0.0.1";
static const int    MC_PORT       = 2416;
static const char * MC_INTRFC     = "192.168.1.7";
static const int    SHAPES_SOURCE = 42;

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

static void ShareableShape_serialize( const ShareableShape * This, ioByteBuffer * target ) {
   unsigned int len = (unsigned int)strlen( This->name ) + 1;
   dcrudByteBuffer_putInt( target, len );
   dcrudByteBuffer_put( target, (byte *)This->name, len );
   dcrudByteBuffer_putDouble( target, This->x );
   dcrudByteBuffer_putDouble( target, This->y );
   dcrudByteBuffer_putDouble( target, This->w );
   dcrudByteBuffer_putDouble( target, This->h );
   dcrudByteBuffer_putDouble( target, This->fill  .red );
   dcrudByteBuffer_putDouble( target, This->fill  .green );
   dcrudByteBuffer_putDouble( target, This->fill  .blue );
   dcrudByteBuffer_putDouble( target, This->fill  .opacity );
   dcrudByteBuffer_putDouble( target, This->stroke.red );
   dcrudByteBuffer_putDouble( target, This->stroke.green );
   dcrudByteBuffer_putDouble( target, This->stroke.blue );
   dcrudByteBuffer_putDouble( target, This->stroke.opacity );
}

static void ShareableShape_unserialize( ioByteBuffer * source ) {
   (void)source;
}

static const int ShareableRect_CLASS_ID = 1;
static const int ShareableEllipse_CLASS_ID = 2;

static dcrudShareable * shapeFactory( int classId ) {
   if(  ( classId == ShareableRect_CLASS_ID    )
      ||( classId == ShareableEllipse_CLASS_ID ))
   {
      return (dcrudShareable *)malloc( sizeof( ShareableShape ));
   }
   fprintf( stderr, "Unexpected class Id: %d\n", classId );
   exit(-1);
}

static void exitHook( void ) {
   WSACleanup();
}

static double nextDouble( double max, double min ) {
   return min + ((double)rand() / RAND_MAX )*( max - min );
}

static ShareableShape * createShape( dcrudIRepository * shapes, int classId ) {
   ShareableShape * shape = (ShareableShape *)malloc( sizeof( ShareableShape ));
   memset( shape, 0 , sizeof( ShareableShape ));
   shape->base.classId     = classId;
   shape->base.set         = (dcrudShareable_set        )ShareableShape_set;
   shape->base.serialize   = (dcrudShareable_serialize  )ShareableShape_serialize;
   shape->base.unserialize = (dcrudShareable_unserialize)ShareableShape_unserialize;
   shape->x                = nextDouble( 540,  0 );
   shape->y                = nextDouble( 400,  0 );
   shape->w                = nextDouble( 100, 40 );
   shape->h                = nextDouble(  80, 20 );
   shape->fill.red         = nextDouble( 255,  0 );
   shape->fill.green       = nextDouble( 255,  0 );
   shape->fill.blue        = nextDouble( 255,  0 );
   shape->fill.opacity     = nextDouble( 255,  0 );
   shape->stroke.red       = nextDouble( 255,  0 );
   shape->stroke.green     = nextDouble( 255,  0 );
   shape->stroke.blue      = nextDouble( 255,  0 );
   shape->stroke.opacity   = nextDouble( 255,  0 );
   dcrudIRepository_create( shapes, (dcrudShareable *)shape );
   return shape;
}

int functionalTest( int argc, char * argv[] ) {
   const char *   address = MC_GROUP;
   unsigned short port    = MC_PORT;
   const char *   intrfc  = MC_INTRFC;
   for( int i = 1; i < argc; ++i ) {
      if( 0 == strcmp( "--address", argv[i] )) {
         address = argv[++i];
      }
      else if( 0 == strcmp( "--port", argv[i] )) {
         port = (unsigned short)atoi( argv[++i] );
      }
      else if( 0 == strcmp( "--interface", argv[i] )) {
         intrfc = argv[++i];
      }
      else {
         fprintf( stderr, "unexpected argument: %s\n", argv[i] );
         exit(-1);
      }
   }
   WORD wVersionRequested = MAKEWORD( 2, 2 );
   WSADATA wsaData;
   int err = WSAStartup( wVersionRequested, &wsaData );
   if( err != 0 ) {
      printf( "WSAStartup failed with error: %d\n", err );
      return 1;
   }
   atexit( exitHook );
   dcrudIRepositoryFactory * repositories =
      dcrudRepositoryFactoryBuilder_join( address, intrfc, port );
   dcrudIRepository * shapes =
      dcrudIRepositoryFactory_getRepository( repositories, SHAPES_SOURCE, true, shapeFactory );
   ShareableShape * rect1 = createShape( shapes, ShareableRect_CLASS_ID );
   ShareableShape * elli1 = createShape( shapes, ShareableEllipse_CLASS_ID );
   ShareableShape * rect2 = createShape( shapes, ShareableRect_CLASS_ID );
   ShareableShape * elli2 = createShape( shapes, ShareableEllipse_CLASS_ID );
   dcrudIRepository_publish( shapes );
   for(;;) {
      rect1->x += 2.0*rand();
      rect1->y += 2.0*rand();
      dcrudIRepository_update( shapes, (dcrudShareable *)rect1 );
      elli1->x += 2.0*rand();
      elli1->y += 2.0*rand();
      dcrudIRepository_update( shapes, (dcrudShareable *)elli1 );
      rect2->x += 2.0*rand();
      rect2->y += 2.0*rand();
      dcrudIRepository_update( shapes, (dcrudShareable *)rect2 );
      elli2->x += 2.0*rand();
      elli2->y += 2.0*rand();
      dcrudIRepository_update( shapes, (dcrudShareable *)elli2 );
      dcrudIRepository_publish( shapes );
      Sleep( 40 );
   }
   printf( "Well done." );
   return 0;
}
