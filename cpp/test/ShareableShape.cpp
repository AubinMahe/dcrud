#include "ShareableShape.hpp"

#include <dcrud/Arguments.hpp>
#include <dcrud/ICache.hpp>
#include <dcrud/IOperation.hpp>
#include <dcrud/IDispatcher.hpp>
#include <dcrud/IProvided.hpp>

#include <sstream>
#include <stdexcept>

const dcrud::ClassID ShareableShape::RectangleClassID( 1, 1, 1, 1 );
const dcrud::ClassID ShareableShape::EllipseClassID  ( 1, 1, 1, 2 );
unsigned int         ShareableShape::Rank = 1;

static ShareableShape * createRectangle() {
   return new ShareableShape( ShareableShape::RectangleClassID );
}

static ShareableShape * createEllipse() {
   return new ShareableShape( ShareableShape::EllipseClassID );
}

static struct OpCreateShape : public dcrud::IOperation {

   virtual dcrud::Arguments * execute(
      dcrud::IParticipant &    participant,
      const dcrud::Arguments & args         )
   {
      dcrud::ClassID clazz;
      if( args.get( "class", clazz )) {
         ShareableShape * shape = new ShareableShape( clazz );
         shape->set( args );
         dcrud::ICache & cache = participant.getCache( 0 );
         cache.create( *shape );
      }
      return 0;
   }

} opCreateShape;

struct OpUpdateShape : public dcrud::IOperation {

   virtual dcrud::Arguments * execute(
      dcrud::IParticipant &    participant,
      const dcrud::Arguments & args         )
   {
      dcrud::Shareable * shareable = 0;
      if( args.get( "shape", shareable )) {
         ShareableShape * shape = dynamic_cast<ShareableShape *>( shareable );
         shape->move();
         dcrud::ICache & cache = participant.getCache( 0 );
         cache.update( *shape );
      }
      return 0;
   }

} opUpdateShape;

static struct OpDeleteShape : public dcrud::IOperation {

   virtual dcrud::Arguments * execute(
      dcrud::IParticipant &    participant,
      const dcrud::Arguments & args         )
   {
      dcrud::Shareable * shareable = 0;
      if( args.get( "shape", shareable )) {
         ShareableShape * shape = dynamic_cast<ShareableShape *>( shareable );
         dcrud::ICache &  cache = participant.getCache( 0 );
         cache.deleTe( *shape );
         cache.publish();
      }
      return 0;
   }

} opDeleteShape;

void ShareableShape::set( const dcrud::Arguments & args ) {
   args.get( "x", x );
   args.get( "y", y );
   args.get( "w", w );
   args.get( "h", h );
}

void ShareableShape::registerClasses( dcrud::IParticipant & participant ) {
   participant.registerLocalFactory( RectangleClassID, (dcrud::localFactory_t)&createRectangle );
   participant.registerLocalFactory( EllipseClassID  , (dcrud::localFactory_t)&createEllipse );
}

void ShareableShape::registerOperations( dcrud::IDispatcher &  dispatcher ) {
   dispatcher
      .provide( "IShapes" )
         .addOperation( "create", opCreateShape )
         .addOperation( "update", opUpdateShape )
         .addOperation( "delete", opDeleteShape );
}

static double nextDouble( double min, double max ) {
   return min + ((double)rand() / RAND_MAX )*( max - min );
}

ShareableShape::ShareableShape( const dcrud::ClassID & classID ) :
   dcrud::Shareable( classID )
{
   std::ostringstream fmt;
   if( classID == RectangleClassID ) {
      fmt << "Rectangle " << ++Rank;
   }
   else if( classID == EllipseClassID ) {
      fmt << "Ellipse " << ++Rank;
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

static const double AREA_MAX_X = 640.0;
static const double AREA_MAX_Y = 480.0;
static const double MOVE       =   2.0;

void ShareableShape::move() {
   bool outOfBounds;
   do {
      outOfBounds = false;
      x += nextDouble( 0.0, 1.0 )*dx;
      y += nextDouble( 0.0, 1.0 )*dy;
      if( x < 0 ) {
         outOfBounds = true;
         dx = +MOVE;
      }
      else if( x+w > AREA_MAX_X ) {
         outOfBounds = true;
         dx = -MOVE;
      }
      if( y < 0 ) {
         outOfBounds = true;
         dy = +MOVE;
      }
      else if( y+h > AREA_MAX_Y ) {
         outOfBounds = true;
         dy = -MOVE;
      }
   } while( outOfBounds );
}

void ShareableShape::serialize( io::ByteBuffer & target ) const {
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

void ShareableShape::unserialize( io::ByteBuffer & source ) {
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
