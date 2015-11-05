#pragma once

#include <crtdbg.h>

int unitTests     ( int argc, char * argv[] );
int functionalTest( int argc, char * argv[] );

int main( int argc, char * argv[] ) {
   _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF );
   //unitTests( argc, argv );
   functionalTest( argc, argv );
   return 0;
}
