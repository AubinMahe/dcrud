#include <dcrud/Network.hpp>
#include "ParticipantImpl.hpp"
#include "NetworkReceiver.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdexcept>
#include <vector>

using namespace dcrud;

#define STRING_SIZE 100

typedef struct Participant_s {

   unsigned short id;
   char           address[16];
   unsigned short port;
   char           subscriptions[STRING_SIZE];

} Participant;

static std::vector<NetworkReceiver *> s_receivers;

IParticipant & Network::join(
   const char *   networkConfFile,
   const char *   intrfc,
   unsigned short id          )
{
   if( !networkConfFile ) {
      throw std::invalid_argument( "Network configuration file can't be null" );
   }
   if( !intrfc ) {
      throw std::invalid_argument( "Network interface name can't be null" );
   }
   FILE * file = fopen( networkConfFile, "rt" );
   if( ! file ) {
      perror( networkConfFile );
      throw std::invalid_argument( networkConfFile );
   }
   char          line[8*1024];
   Participant   conf[65536];
   unsigned      count     = 0;
   Participant * publisher = 0;
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
      char buffer[1024];
      snprintf( buffer, sizeof( buffer ), "Participant '%d' not found in %s\n",
         id, networkConfFile );
      throw std::invalid_argument( buffer );
   }
   ParticipantImpl * participant =
      new ParticipantImpl( publisher->id, publisher->address, publisher->port, intrfc );
   char * idSub = strtok( publisher->subscriptions, "," );
   while( idSub ) {
      unsigned int  index      = (unsigned int)atoi( idSub ) - 1;
      Participant * subscriber = &conf[index];
      s_receivers.push_back(
         new NetworkReceiver( *participant, subscriber->address, subscriber->port, intrfc ));
      idSub = strtok( NULL, "," );
   }
   return *participant;
}
