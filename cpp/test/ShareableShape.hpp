#pragma once

#include <dcrud/Shareable.hpp>
#include <dcrud/IParticipant.hpp>
#include <dcrud/Arguments.hpp>

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

class ShareableShape : public dcrud::Shareable {
public:

   static const dcrud::ClassID RectangleClassID;
   static const dcrud::ClassID EllipseClassID;

   static void registerClasses   ( dcrud::IParticipant & participant );
   static void registerOperations( dcrud::IDispatcher &  dispatcher  );

public:

   ShareableShape( const dcrud::ClassID & classID );

   void set( const dcrud::Arguments & args );

   void move();

   virtual void serialize( io::ByteBuffer & target ) const;

   virtual void unserialize( io::ByteBuffer & source );

private:

   static unsigned int Rank;

   std::string name;
   double      x;
   double      y;
   double      w;
   double      h;
   FxColor     fill;
   FxColor     stroke;
   double      dx;
   double      dy;
};
