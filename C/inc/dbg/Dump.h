#pragma once

#include <util/types.h>
#include <stdio.h>

void dump_range( FILE * target, const byte * buffer, size_t from, size_t to );

void dump( FILE * target, const byte * buffer, size_t length );
