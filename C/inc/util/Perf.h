#pragma once
#include <util/types.h>

void utilPerf_enable ( bool enabled );
void utilPerf_record ( const char * attribute, uint64_t elapsed );
void utilPerf_display();
