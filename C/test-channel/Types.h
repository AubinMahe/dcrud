#pragma once

#include <util/Status.h>

#define channelTestFORNAME_SIZE 80
#define channelTestNAME_SIZE    80

typedef char channelTestForname[channelTestFORNAME_SIZE];
utilStatus channelTestForname_set( channelTestForname target, const char * source );

typedef char channelTestName[channelTestNAME_SIZE];
utilStatus channelTestName_set( channelTestName target, const char * source );
