#pragma once

#include <dcrud/Shareable.hpp>
#include <dcrud/IParticipant.hpp>

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

struct ShareableShape : public dcrud::Shareable {

   static const dcrud::ClassID RectangleClassID;
   static const dcrud::ClassID EllipseClassID;
   static unsigned int         Rank;

   static void registerClasses   ( dcrud::IParticipant & participant );
   static void registerOperations( dcrud::IDispatcher &  dispatcher  );

   std::string name;
   double      x;
   double      y;
   double      w;
   double      h;
   FxColor     fill;
   FxColor     stroke;
   double      dx;
   double      dy;

public:

   ShareableShape( const dcrud::ClassID & classID );

   void move();

   virtual void serialize( io::ByteBuffer & target ) const;

   virtual void unserialize( io::ByteBuffer & source );
};
