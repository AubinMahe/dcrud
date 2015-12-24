#include <dcrud/Network.hpp>
#include <dcrud/ICache.hpp>
#include <dcrud/IParticipant.hpp>
#include <dcrud/IDispatcher.hpp>

#include <os/System.h>

#include "ShareableShape.hpp"

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

class ShapesSample {
public:

   static const unsigned int LOOP_COUNT = 10000U;

   ShapesSample() :
      _participant( dcrud::Network::join( 2, "224.0.0.3", 2417, "192.168.1.7" )),
      _cache      ( _participant->getDefaultCache()),
      _dispatcher ( _participant->getDispatcher())
   {
      ShareableShape::registerClasses   ( *_participant );
      ShareableShape::registerOperations( _dispatcher  );
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
      delete _participant;
   }

   void run() {
      _cache.create( *new ShareableShape( ShareableShape::RectangleClassID ));
      _cache.create( *new ShareableShape( ShareableShape::EllipseClassID ));
      _cache.create( *new ShareableShape( ShareableShape::RectangleClassID ));
      _cache.create( *new ShareableShape( ShareableShape::EllipseClassID ));
      std::cout << "Publish every 40 ms, " << LOOP_COUNT << " times." << std::endl;
      std::set<dcrud::Shareable *> snapshot;
      for( unsigned i = 0; i < LOOP_COUNT; ++i ) {
         _cache.publish();
         osSystem_sleep( 40U );
         _cache.values( snapshot );
         for( dcrud::shareablesIter_t it = snapshot.begin(); it != snapshot.end(); ++it ) {
            ShareableShape * shape = dynamic_cast<ShareableShape *>( *it );
            shape->move();
            _cache.update( *shape );
         }
         _dispatcher.handleRequests();
      }
   }

private:

   dcrud::IParticipant * _participant;
   dcrud::ICache &       _cache;
   dcrud::IDispatcher &  _dispatcher;
};

int main( void ) {
   try {
      ShapesSample().run();
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
