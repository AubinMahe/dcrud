#include "Types.h"

#include <string.h>

utilStatus channelTestForname_set( channelTestForname target, const char * source ) {
	if( ! source ) {
		source = "";
	}
	strncpy( target, source, channelTestFORNAME_SIZE );
	target[channelTestFORNAME_SIZE-1] = '\0';
	return UTIL_STATUS_NO_ERROR;
}

utilStatus channelTestName_set( channelTestName target, const char * source ) {
	if( ! source ) {
		source = "";
	}
   strncpy( target, source, channelTestNAME_SIZE );
   target[channelTestNAME_SIZE-1] = '\0';
   return UTIL_STATUS_NO_ERROR;
}
