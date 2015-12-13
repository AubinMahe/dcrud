#include <dcrud/Network.hpp>
#include <dcrud/ClassID.hpp>
#include <dcrud/Shareable.hpp>
#include <dcrud/IOperation.hpp>
#include <dcrud/ICache.hpp>
#include <dcrud/IParticipant.hpp>
#include <dcrud/IDispatcher.hpp>
#include <dcrud/IProvided.hpp>

#include <io/ByteBuffer.hpp>

#include <os/System.h>
#include <util/CheckSysCall.h>

#include <string>
#include <map>
#include <sstream>
#include <stdexcept>
#include <algorithm>

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

#define LOOP_COUNT 10000U

/*
#define PRINT_SERIALIZE
#define PRINT_TIMING
*/
dcrud::ClassID rectangleClassID( 1, 1, 1, 1 );
dcrud::ClassID ellipseClassID  ( 1, 1, 1, 2 );

struct FxColor {

   double red;
   double green;
   double blue;
   double opacity;

   void set( const FxColor & source ) {
      red     = source.red;
      green   = source.green;
      blue    = source.blue;
      opacity = source.opacity;
   }
};

static unsigned int g_rank;

static double nextDouble( double max, double min ) {
   return min + ((double)rand() / RAND_MAX )*( max - min );
}

struct ShareableShape : public dcrud::Shareable {

   std::string name;
   double      x;
   double      y;
   double      w;
   double      h;
   FxColor     fill;
   FxColor     stroke;
   double      dx;
   double      dy;

   ShareableShape( const dcrud::ClassID & classID ) :
      dcrud::Shareable( classID )
   {
      std::ostringstream fmt;
      if( classID == rectangleClassID ) {
         fmt << "Rectangle " << ++g_rank;
      }
      else if( classID == ellipseClassID ) {
         fmt << "Ellipse " << ++g_rank;
      }
      else {
         throw std::invalid_argument( classID.toString());
      }
      name           = fmt.str();
      x              = nextDouble( 540.0,  0.0 );
      y              = nextDouble( 400.0,  0.0 );
      w              = nextDouble( 100.0, 40.0 );
      h              = nextDouble(  80.0, 20.0 );
      fill.red       = nextDouble(   1.0,  0.0 );
      fill.green     = nextDouble(   1.0,  0.0 );
      fill.blue      = nextDouble(   1.0,  0.0 );
      fill.opacity   = nextDouble(   1.0,  0.0 );
      stroke.red     = nextDouble(   1.0,  0.0 );
      stroke.green   = nextDouble(   1.0,  0.0 );
      stroke.blue    = nextDouble(   1.0,  0.0 );
      stroke.opacity = nextDouble(   1.0,  0.0 );
      dx             = 1.0;
      dy             = 1.0;
   }

   void set( const ShareableShape & source ) {
      name = source.name;
      x    = source.x;
      y    = source.y;
      w    = source.w;
      h    = source.h;
      fill  .set( source.fill   );
      stroke.set( source.stroke );
   }

   virtual void serialize( io::ByteBuffer & target ) const {
      target.putString( name.c_str());
      target.putDouble( x );
      target.putDouble( y );
      target.putDouble( w );
      target.putDouble( h );
      target.putDouble( fill  .red );
      target.putDouble( fill  .green );
      target.putDouble( fill  .blue );
      target.putDouble( fill  .opacity );
      target.putDouble( stroke.red );
      target.putDouble( stroke.green );
      target.putDouble( stroke.blue );
      target.putDouble( stroke.opacity );
   #ifdef PRINT_SERIALIZE
      fprintf( stderr, "-- Serialize --\n" );
      fprintf( stderr, "name          : %s\n", name );
      fprintf( stderr, "x             : %f\n", x );
      fprintf( stderr, "y             : %f\n", y );
      fprintf( stderr, "w             : %f\n", w );
      fprintf( stderr, "h             : %f\n", h );
      fprintf( stderr, "fill  .red    : %f\n", fill  .red     );
      fprintf( stderr, "fill  .green  : %f\n", fill  .green   );
      fprintf( stderr, "fill  .blue   : %f\n", fill  .blue    );
      fprintf( stderr, "fill  .opacity: %f\n", fill  .opacity );
      fprintf( stderr, "stroke.red    : %f\n", stroke.red     );
      fprintf( stderr, "stroke.green  : %f\n", stroke.green   );
      fprintf( stderr, "stroke.blue   : %f\n", stroke.blue    );
      fprintf( stderr, "stroke.opacity: %f\n", stroke.opacity );
   #endif
   }

   virtual void unserialize( io::ByteBuffer & source ) {
      name           = source.getString();
      x              = source.getDouble();
      y              = source.getDouble();
      w              = source.getDouble();
      h              = source.getDouble();
      fill  .red     = source.getDouble();
      fill  .green   = source.getDouble();
      fill  .blue    = source.getDouble();
      fill  .opacity = source.getDouble();
      stroke.red     = source.getDouble();
      stroke.green   = source.getDouble();
      stroke.blue    = source.getDouble();
      stroke.opacity = source.getDouble();
   }
};

static ShareableShape * createRectangle() {
   return new ShareableShape( rectangleClassID );
}

static ShareableShape * createEllipse() {
   return new ShareableShape( ellipseClassID );
}

//static double areaMaxX = 640.0;
//static double areaMaxY = 480.0;
#define MOVE 2.0

static bool moveShape( dcrud::Shareable * shareable ) {
/*
   collMapPair *    pair      = (collMapPair *   )context->item;
   dcrudICache      cache     = (dcrudICache     )context->user;
   dcrud = (dcrudShareable  )pair->value;
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
*/
   (void)shareable;
   return true;
}

static bool removeFromCache( dcrud::Shareable * shareable ) {
   /*
   collMapPair *  pair      = (collMapPair *   )context->item;
   dcrudICache    cache     = (dcrudICache     )context->user;
   dcrudShareable shareable = (dcrudShareable  )pair->value;
   dcrudICache_delete( cache, shareable );
   */
   (void)shareable;
   return true;
}

static bool deleteShape( dcrud::Shareable * shareable ) {
   delete shareable;
   return true;
}

class ShapesFactory : public dcrud::IOperation {
private:

   dcrud::IParticipant & _participant;
   dcrud::ICache &       _cache;

public:

   ShapesFactory( dcrud::IParticipant & participant ) :
      _participant( participant ),
      _cache      ( participant.getCache( 0 ))
   {}

   virtual void execute( const dcrud::Arguments & in, dcrud::args_t & out ) {
      const dcrud::ClassID * clazz;
      if( in.get( "class", clazz )) {
         ShareableShape * shape = new ShareableShape( *clazz );
         in.get( "x", shape->x );
         in.get( "y", shape->y );
         in.get( "w", shape->w );
         in.get( "h", shape->h );
         _cache.create( *shape );
      }
      out.clear(); /* No out parameter */
   }
};

int main( int argc, char * argv[] ) {
   int          pubId       = 0;
   const char * intrfc;

   for( int i = 2; i < argc; ++i ) {
      if( 0 == strcmp( argv[i], "--pub-id" )) {
         pubId = (unsigned short)atoi( argv[++i] );
      }
      else if( 0 == strcmp( argv[i], "--interface" )) {
         intrfc = argv[++i];
      }
   }
   if( pubId < 1 ) {
      fprintf( stderr, "%s --pub-id <publisher-id> is mandatory\n", argv[0] );
      exit(-1);
   }
   if( ! intrfc ) {
      fprintf( stderr, "%s --interface <ipv4> is mandatory\n", argv[0] );
      exit(-1);
   }
   try {
      dcrud::IParticipant & participant =
         dcrud::Network::join( "network.cfg", intrfc, (unsigned short)pubId );
#ifdef PRINT_TIMING
      static uint64_t prev = osSystem_nanotime();
#endif
      participant.registerClass( rectangleClassID, (dcrud::factory_t)&createRectangle );
      participant.registerClass( ellipseClassID  , (dcrud::factory_t)&createEllipse   );
      dcrud::ICache & cache = participant.createCache();
      cache.create( *new ShareableShape( rectangleClassID ));
      cache.create( *new ShareableShape( ellipseClassID   ));
      cache.create( *new ShareableShape( rectangleClassID ));
      cache.create( *new ShareableShape( ellipseClassID   ));
      dcrud::IDispatcher & dispatcher = participant.getDispatcher();
      dcrud::IProvided & shapesFactory = dispatcher.provide( "IShapesFactory" );
      ShapesFactory      shapesFactoryCreate( participant );
      shapesFactory.addOperation( "create", shapesFactoryCreate );
      std::cout << "Publish every 40 ms, " << LOOP_COUNT << " times." << std::endl;
      std::set<dcrud::Shareable *> snapshot;
      for( unsigned i = 0; i < LOOP_COUNT; ++i ) {
   #ifdef PRINT_TIMING
         uint64_t now = osSystem_nanotime();
         fprintf( stderr, "-- Publish, delta = %7.2f ------------------------------------------\n",
            ((double)(now-prev))/1000000.0 );
         prev = now;
   #endif
         cache.publish();
         osSystem_sleep( 40U );
         cache.values( snapshot );
         std::for_each( snapshot.begin(), snapshot.end(), moveShape );
         dispatcher.handleRequests();
      }
      cache.values( snapshot );
      std::for_each( snapshot.begin(), snapshot.end(), removeFromCache );
      cache.publish();
      std::for_each( snapshot.begin(), snapshot.end(), deleteShape );
      dcrud::Network::leave( participant );
      printf( "Well done.\n" );
   }
   catch( const std::exception & x ) {
      std::cerr << x.what() << std::endl;
   }
   catch( ... ) {
      std::cerr << "Exception '...' catched!" << std::endl;
   }
   return 0;
}
