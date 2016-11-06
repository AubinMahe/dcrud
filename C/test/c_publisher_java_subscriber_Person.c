/*
 *
 * TEST 5
 *
 */
#include <dcrud/Network.h>
#include <os/System.h>
#include <util/CmdLine.h>
#include <util/Pool.h>
#include <io/NetworkInterfaces.h>

#include "Person.h"
#include "Settings.h"
#include "StaticRegistry.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static dcrudArguments exitSrvc( dcrudIParticipant participant, dcrudArguments args ) {
   dcrudNetwork_leave();
   (void)participant;
   (void)args;
   return NULL;
}

utilStatus c_publisher_java_subscriber_Person( const utilCmdLine cmdLine ) {
   ioInetSocketAddress p1, p2;
   dcrudIParticipant   participant;
   utilStatus          status = UTIL_STATUS_NO_ERROR;

   CHK(__FILE__,__LINE__,ioInetSocketAddress_init( &p1, MCAST_ADDRESS, 2417 ))
   CHK(__FILE__,__LINE__,ioInetSocketAddress_init( &p2, MCAST_ADDRESS, 2416 ))
   CHK(__FILE__,__LINE__,dcrudNetwork_join( 2, &p1, NETWORK_INTERFACE, &participant ))
   if( participant ) {
      dcrudIDispatcher     dispatcher;
      dcrudIProvided       monitor;
      dcrudLocalFactory *  localFactory;
      dcrudRemoteFactory * remoteFactory;
      bool                 dumpReceivedBuffer;
      dcrudIRegistry       registry = NULL;
      CHK(__FILE__,__LINE__,getStaticRegistry( &registry ))
      CHK(__FILE__,__LINE__,utilCmdLine_getBoolean( cmdLine, "dump-received-buffer", &dumpReceivedBuffer ))
      CHK(__FILE__,__LINE__,Person_initFactories( participant, &localFactory, &remoteFactory ))
      CHK(__FILE__,__LINE__,dcrudIParticipant_listen( participant, registry, NETWORK_INTERFACE, dumpReceivedBuffer ))
      CHK(__FILE__,__LINE__,dcrudIParticipant_getDispatcher( participant, &dispatcher ))
      CHK(__FILE__,__LINE__,dcrudIDispatcher_provide( dispatcher, "IMonitor", &monitor ))
      CHK(__FILE__,__LINE__,dcrudIProvided_addOperation(
         monitor, "exit", participant, (dcrudIOperation)exitSrvc ))
      while( dcrudNetwork_isAlive()) {
         osSystem_sleep( 10U );
         CHK(__FILE__,__LINE__,dcrudIDispatcher_handleRequests( dispatcher ))
      }
      CHK(__FILE__,__LINE__,releaseStaticRegistry( &registry ))
      CHK(__FILE__,__LINE__,Person_releaseFactories())
   }
   else {
      fprintf( stderr, "Unable to join network.\n" );
   }
   return status;
}
