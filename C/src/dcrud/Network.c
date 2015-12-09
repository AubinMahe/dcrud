#include <dcrud/Network.h>
#include <coll/List.h>
#include <stdio.h>
#include <sys/types.h>

#include "INetworkReceiver.h"
#include "ParticipantImpl.h"

#ifdef __linux__
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
#elif _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <winsock2.h>
#  include <ws2tcpip.h>
#  include <mswsock.h>
#endif

#define STRING_SIZE 100

typedef struct Participant_s {

   unsigned short id;
   char           address[16];
   unsigned short port;
   char           subscriptions[STRING_SIZE];

} Participant;

static INetworkReceiver s_receivers[65536];
static unsigned short   s_receiversCount;

dcrudIParticipant dcrudNetwork_join(
   const char *   networkConfFile,
   const char *   intrfc,
   unsigned short id       )
{
   ParticipantImpl * retVal = NULL;

   if( !networkConfFile ) {
      fprintf( stderr, "Network configuration file can't be null\n" );
   }
   else if( !intrfc ) {
      fprintf( stderr, "Network interface name can't be null\n" );
   }
   else {
      FILE * file = fopen( networkConfFile, "rt" );
      if( ! file ) {
         perror( networkConfFile );
      }
      else {
         char          line[8*1024];
         Participant   conf[65536];
         unsigned      count     = 0;
         Participant * publisher = NULL;
         while( fgets( line, sizeof( line ), file )) {
            char * s = strtok( line, " \t" );
            if( !s || *s == '#' ) {
               continue;
            }
            conf[count].id = (unsigned short)atoi( s );
            s = strtok( NULL, " \t" );
            strncpy( conf[count].address, s, 16 );
            s = strtok( NULL, " \t" );
            conf[count].port = (unsigned short)atoi( s );
            s = strtok( NULL, " \t" );
            strncpy( conf[count].subscriptions, s, STRING_SIZE );
            if( conf[count].id == id ) {
               publisher = &conf[count];
            }
            ++count;
         }
         fclose( file );
         if( !publisher ) {
            fprintf( stderr, "Participant '%d' not found in %s\n", id, networkConfFile );
         }
         else {
            if( DCRUD_NO_ERROR == ParticipantImpl_new(
                  publisher->id, publisher->address, publisher->port, intrfc, &retVal ))
            {
               char * id = strtok( publisher->subscriptions, "," );
               while( id ) {
                  unsigned int  index      = (unsigned int)atoi( id ) - 1;
                  Participant * subscriber = &conf[index];
                  s_receivers[s_receiversCount++] =
                     INetworkReceiver_new(
                        retVal, subscriber->address, subscriber->port, intrfc );
                  id = strtok( NULL, "," );
               }
            }
            else {
               ParticipantImpl_delete( &retVal );
            }
         }
      }
   }
   return (dcrudIParticipant)retVal;
}

void dcrudNetwork_leave( dcrudIParticipant * self ) {
   unsigned short i;

   ParticipantImpl_delete((ParticipantImpl **)self);
   for( i = 0; ; ++i ) {
      if( s_receivers[i] ) {
         INetworkReceiver_delete( &s_receivers[i] );
      }
      else {
         break;
      }
   }
}
