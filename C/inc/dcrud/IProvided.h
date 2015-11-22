#pragma once
#include <dcrud/IOperation.h>

UTIL_ADT( dcrudIProvided );

bool dcrudIProvided_addOperation( const char * operationName, dcrudIOperation executor );
