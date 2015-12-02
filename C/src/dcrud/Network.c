#include <dcrud/Network.h>
#include <coll/List.h>
#include <stdio.h>
#include <sys/types.h>
#include <expat.h>

#include "INetworkReceiver.h"
#include "ParticipantImpl.h"

#ifdef __linux__
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
#elif _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <winsock2.h>
#  include <Ws2tcpip.h>
#  include <mswsock.h>
#endif

typedef struct Participant_s {

   unsigned short id;
   char *         name;
   char *         address;
   unsigned short port;
   collList       subscriptions;

} Participant;

static void XmlEvtHndlr_start( void * data, const char *el, const char **attr ) {
   static Participant * p;
   collMap participants = (collMap)data;
   int      i;

   if( 0 == strcmp( el, "participant" )) {
      p = (Participant *)malloc( sizeof( Participant ));
      p->subscriptions = collList_new();
      for( i = 0; attr[i]; i += 2 ) {
         if( 0 == strcmp( attr[i], "id" )) {
            p->id      = (unsigned short)atoi( attr[i+1] );
         }
         else if( 0 == strcmp( attr[i], "name" )) {
            p->name    = strdup( attr[i+1] );
         }
         else if( 0 == strcmp( attr[i], "address" )) {
            p->address = strdup( attr[i+1] );
         }
         else if( 0 == strcmp( attr[i], "port" )) {
            p->port    = (unsigned short)atoi( attr[i+1] );
         }
      }
      collMap_put( participants, p->name, p, NULL );
   }
   else if( 0 == strcmp( el, "subscribe" )) {
      for( i = 0; attr[i]; i += 2 ) {
         if( 0 == strcmp( attr[i], "to" )) {
            collList_add( p->subscriptions, strdup( attr[i+1] ));
         }
      }
   }
}

static bool deletePublishers( collForeach * context ) {
   collMapPair * pair  = (collMapPair *)context->item;
   Participant * pConf = (Participant *)pair->value;
   char **       subs  = (char **)collList_values( pConf->subscriptions );
   unsigned int  size  = collList_size( pConf->subscriptions );
   unsigned int  i;
   free( pConf->name );
   free( pConf->address );
   for( i = 0; i < size; ++i ) {
      free( subs[i] );
   }
   collList_delete( &( pConf->subscriptions ));
   free( pConf );
   return true;
}

static INetworkReceiver s_receivers[65536];

dcrudIParticipant dcrudNetwork_join(
   const char * networkConfFile,
   const char * intrfc,
   const char * name    )
{
   ParticipantImpl * retVal = NULL;

   if( !networkConfFile ) {
      fprintf( stderr, "Network configuration file can't be null\n" );
   }
   else if( !intrfc ) {
      fprintf( stderr, "Network interface name can't be null\n" );
   }
   else if( !name ) {
      fprintf( stderr, "Participant name can't be null\n" );
   }
   else {
      FILE * file = fopen( "network.xml", "rt" );
      if( ! file ) {
         perror( "network.xml" );
      }
      else {
         char       line[8*1024];
         collMap    conf   = collMap_new((collComparator)collStringComparator );
         XML_Parser parser = XML_ParserCreate( "UTF-8" );
         bool       ok     = true;
         XML_SetUserData( parser, conf );
         XML_SetElementHandler( parser, XmlEvtHndlr_start, NULL );
         while( fgets( line, sizeof( line ), file )) {
            if( ! XML_Parse( parser, line, (int)strlen( line ), 0 )) {
               fprintf(stderr, "Parse error at line %lu:\n%s\n",
                  XML_GetCurrentLineNumber( parser ),
                  XML_ErrorString( XML_GetErrorCode( parser )));
               ok = false;
               break;
            }
         }
         XML_Parse( parser, "", 0, 1 );
         XML_ParserFree( parser );
         fclose( file );
         if( ok ) {
            Participant * publisher = (Participant *)collMap_get( conf, (collMapKey)name );
            if( !publisher ) {
               fprintf( stderr, "Participant '%s' not found in %s\n", name, networkConfFile );
            }
            else {
               char **      subs = (char **)collList_values( publisher->subscriptions );
               unsigned int size = collList_size( publisher->subscriptions );
               unsigned int i;
               if( DCRUD_NO_ERROR == ParticipantImpl_new(
                     publisher->id, publisher->address, publisher->port, intrfc, &retVal ))
               {
                  for( i = 0; i < size; ++i ) {
                     Participant * subscriber =
                        (Participant *)collMap_get( conf, (collMapKey)subs[i] );
                     s_receivers[i] =
                        INetworkReceiver_new(
                           retVal, subscriber->address, subscriber->port, intrfc );
                  }
               }
               else {
                  ParticipantImpl_delete( &retVal );
               }
            }
         }
         collMap_foreach( conf, (collForeachFunction)deletePublishers, NULL );
         collMap_delete( &conf );
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
