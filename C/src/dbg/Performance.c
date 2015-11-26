#include <dbg/Performance.h>

#include <coll/List.h>
#include <coll/Map.h>

#include <stdio.h>
#include <string.h>

static collMap s_samplesByAttribute;
static bool    s_enabled = false;

void dbgPerformance_enable( bool enabled ) {
   s_enabled = enabled;
}

void dbgPerformance_record( const char * attribute, uint64_t elapsed ) {
   collList          samples;
   uint64_t *        sample;
   collMapKey key;

   if( ! s_enabled ) {
      return;
   }
   key     = strdup( attribute );
   samples = (collList)collMap_get( s_samplesByAttribute, key );
   sample  = (uint64_t *)malloc( sizeof( uint64_t ));
   *sample = elapsed;
   if( samples == NULL ) {
      collMap_put( s_samplesByAttribute, key, samples = collList_new(), NULL );
   }
   collList_add( samples, sample );
}

typedef struct MinMaxAvg_s {

   double min;
   double max;
   double sum;

} MinMaxAvg;

static bool computeSample( collForeach * context ) {
   MinMaxAvg * minMaxAvg = (MinMaxAvg *)context->user;
   uint64_t *  sample    = (uint64_t * )context->item;

   if( *sample < minMaxAvg->min ) {
      minMaxAvg->min = (double)*sample;
   }
   if( *sample > minMaxAvg->max ) {
      minMaxAvg->max = (double)*sample;
   }
   minMaxAvg->sum += (double)*sample;
   return true;
}

static bool saveSamplesToText( collForeach * context ) {
   FILE *               out       = (FILE *              )context->user;
   collMapPair * pair      = (collMapPair *)context->item;
   const char *         attribute = (const char *        )pair->key;
   collList             samples   = (collList            )pair->value;
   MinMaxAvg            minMaxAvg = { 1.0E30, 0.0, 0.0 };
   double               avg       = 0.0;

   collList_foreach( samples, computeSample, &minMaxAvg );
   avg = minMaxAvg.sum / collList_size( samples );
   fprintf( out, "| %-10s | %9.2f | %9.2f | %9.2f |\n",
      attribute, minMaxAvg.min, minMaxAvg.max, avg );
   return true;
}

static bool saveSamplesToGnuPlot( collForeach * context ) {
   collMapPair * pair           = (collMapPair *)context->item;
   const char *         attribute      = (const char *        )pair->key;
   collList             samples        = (collList            )pair->value;
   char                 filename[1024] = "";
   FILE *               out;
   unsigned int         count;
   unsigned int         index;

   strcat( filename, attribute );
   strcat( filename, ".dat" );
   out   = fopen( filename, "wt" );
   count = collList_size( samples );
   for( index = 0; index < count; ++index ) {
      uint64_t * measure = collList_get( samples, index );
      double   value   = (double)*measure;
      fprintf( out, "%9.2f\n", value / 1000.0 );
   }
   return true;
}

void dbgPerformance_saveToDisk( void ) {
   FILE * out;
   if( ! s_enabled ) {
      return;
   }

   out = fopen( "perfo.txt", "wt" );
   fprintf( out, "+------------+-----------+-----------+-----------+\n" );
   fprintf( out, "| Attribute  |    Min µs |    Max µs |    Avg µs |\n" );
   fprintf( out, "+------------+-----------+-----------+-----------+\n" );
   collMap_foreach( s_samplesByAttribute, saveSamplesToText, out );
   fprintf( out, "+------------+-----------+-----------+-----------+\n" );
   fclose( out );
   collMap_foreach( s_samplesByAttribute, saveSamplesToGnuPlot, out );
}
