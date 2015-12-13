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

static const double       AREA_MAX_X = 640.0;
static const double       AREA_MAX_Y = 480.0;
static const double       MOVE       =   2.0;
static const unsigned int LOOP_COUNT = 10000U;

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

static double nextDouble( double min, double max ) {
   return min + ((double)rand() / RAND_MAX )*( max - min );
}

struct ShareableShape : public dcrud::Shareable {

   static unsigned int g_rank;

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
      x              = nextDouble(  0.0, 540.0 );
      y              = nextDouble(  0.0, 400.0 );
      w              = nextDouble( 40.0, 100.0 );
      h              = nextDouble( 20.0,  80.0 );
      fill.red       = nextDouble(  0.0,   1.0 );
      fill.green     = nextDouble(  0.0,   1.0 );
      fill.blue      = nextDouble(  0.0,   1.0 );
      fill.opacity   = nextDouble(  0.0,   1.0 );
      stroke.red     = nextDouble(  0.0,   1.0 );
      stroke.green   = nextDouble(  0.0,   1.0 );
      stroke.blue    = nextDouble(  0.0,   1.0 );
      stroke.opacity = nextDouble(  0.0,   1.0 );
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

unsigned int ShareableShape::g_rank = 1;

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

struct ShapesSample {

   static ShareableShape * createRectangle() {
      return new ShareableShape( rectangleClassID );
   }

   static ShareableShape * createEllipse() {
      return new ShareableShape( ellipseClassID );
   }

   bool moveShape( dcrud::Shareable * shareable ) {
      ShareableShape * shape = dynamic_cast<ShareableShape *>( shareable );
      bool             outOfBounds;
      do {
         outOfBounds = false;
         shape->x += nextDouble( 0.0, 1.0 )*shape->dx;
         shape->y += nextDouble( 0.0, 1.0 )*shape->dy;
         if( shape->x < 0 ) {
            outOfBounds = true;
            shape->dx = +MOVE;
         }
         else if( shape->x+shape->w > AREA_MAX_X ) {
            outOfBounds = true;
            shape->dx = -MOVE;
         }
         if( shape->y < 0 ) {
            outOfBounds = true;
            shape->dy = +MOVE;
         }
         else if( shape->y+shape->h > AREA_MAX_Y ) {
            outOfBounds = true;
            shape->dy = -MOVE;
         }
      } while( outOfBounds );
      _cache.update( *shape );
      return true;
   }

   dcrud::IParticipant & _participant;
   dcrud::ICache &       _cache;
   dcrud::IDispatcher &  _dispatcher;
   ShapesFactory         _opCreateShape;

   ShapesSample( unsigned short publisherId, const char * intrfc ) :
      _participant  ( dcrud::Network::join( "network.cfg", intrfc, publisherId )),
      _cache        ( _participant.createCache()),
      _dispatcher   ( _participant.getDispatcher()),
      _opCreateShape( _participant )
   {
      _participant.registerClass( rectangleClassID, (dcrud::factory_t)&createRectangle );
      _participant.registerClass( ellipseClassID  , (dcrud::factory_t)&createEllipse   );
      _cache.create( *new ShareableShape( rectangleClassID ));
      _cache.create( *new ShareableShape( ellipseClassID   ));
      _cache.create( *new ShareableShape( rectangleClassID ));
      _cache.create( *new ShareableShape( ellipseClassID   ));
      _dispatcher.provide( "IShapesFactory" )
         .addOperation( "create", _opCreateShape );
   }

   ~ ShapesSample() {
      dcrud::shareables_t snapshot;
      _cache.values( snapshot );
      for( dcrud::shareablesIter_t it = snapshot.begin(); it != snapshot.end(); ++it ) {
         _cache.deleTe( **it );
      }
      _cache.publish();
      for( dcrud::shareablesIter_t it = snapshot.begin(); it != snapshot.end(); ++it ) {
         delete *it;
      }
      dcrud::Network::leave( _participant );
   }

   void run() {
      std::cout << "Publish every 40 ms, " << LOOP_COUNT << " times." << std::endl;
      std::set<dcrud::Shareable *> snapshot;
      for( unsigned i = 0; i < LOOP_COUNT; ++i ) {
         _cache.publish();
         osSystem_sleep( 40U );
         _cache.values( snapshot );
         for( dcrud::shareablesIter_t it = snapshot.begin(); it != snapshot.end(); ++it ) {
            moveShape( *it );
         }
         _dispatcher.handleRequests();
      }
   }
};

int main( int argc, char * argv[] ) {
   unsigned short publisherId = 0;
   const char * intrfc;

   for( int i = 2; i < argc; ++i ) {
      if( 0 == strcmp( argv[i], "--pub-id" )) {
         publisherId = (unsigned short)atoi( argv[++i] );
      }
      else if( 0 == strcmp( argv[i], "--interface" )) {
         intrfc = argv[++i];
      }
   }
   if( publisherId < 1 ) {
      fprintf( stderr, "%s --pub-id <publisher-id> is mandatory\n", argv[0] );
      exit(-1);
   }
   if( ! intrfc ) {
      fprintf( stderr, "%s --interface <ipv4> is mandatory\n", argv[0] );
      exit(-1);
   }
   try {
      ShapesSample( publisherId, intrfc ).run();
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
