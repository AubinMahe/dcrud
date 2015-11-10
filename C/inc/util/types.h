#pragma once
#include <stdlib.h>

#ifndef __cplusplus
typedef int bool;
#define false 0
#define true (!false)
#endif

typedef unsigned char byte;

#define DCRUD_ADT(T) typedef struct T##_s { int unused; } * T
