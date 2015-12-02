#pragma once
#include <util/types.h>

extern bool utilCheckSysCall(
   bool         ok,
   const char * file,
   int          line,
   const char * format, ... );
