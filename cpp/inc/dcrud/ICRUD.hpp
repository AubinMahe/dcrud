#pragma once

namespace dcrud {

   class Arguments;

   class ICRUD {
   public:

      virtual ~ ICRUD( void ) {}

      virtual void create( const Arguments & how ) = 0;
      virtual void update( Shareable & what, const Arguments & how ) = 0;
      virtual void deleTe( Shareable & what ) = 0;
   };
}
