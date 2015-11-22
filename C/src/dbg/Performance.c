#include <dbg/Performance.h>

#include <coll/MapVoidPtr.h>
#include <coll/List.h>

#include <stdio.h>
#include <string.h>

static collMapVoidPtr s_samplesByAttribute;
static bool           s_enabled = false;

void Performance_enable( bool enabled ) {
   s_enabled = enabled;
}

void Performance_record( const char * attribute, uint64_t elapsed ) {
   collList          samples;
   uint64_t *        sample;
   collMapVoidPtrKey key;

   if( ! s_enabled ) {
      return;
   }
   key     = strdup( attribute );
   samples = (collList)collMapVoidPtr_get( s_samplesByAttribute, key );
   sample  = (uint64_t *)malloc( sizeof( uint64_t ));
   *sample = elapsed;
   if( samples == NULL ) {
      collMapVoidPtr_put( s_samplesByAttribute, key, samples = collList_new(), NULL );
   }
   collList_add( samples, sample );
}

typedef struct MinMaxAvg_s {

   double min;
   double max;
   double sum;

} MinMaxAvg;

static bool computeSample( collForeach * context ) {
   uint64_t *  sample = (uint64_t *)context->item;
   MinMaxAvg * minMaxAvg = (MinMaxAvg *)context->user;

   if( *sample < minMaxAvg->min ) {
      minMaxAvg->min = (double)*sample;
   }
   if( *sample > minMaxAvg->max ) {
      minMaxAvg->max = (double)*sample;
   }
   minMaxAvg->sum += (double)*sample;
   return true;
}

static bool saveSamplesToText( collMapVoidPtrForeach * context ) {
   const char * attribute = (const char *)context->key;
   collList     samples   = (collList    )context->value;
   FILE *       out       = (FILE *      )context->user;
   MinMaxAvg    minMaxAvg = { 1.0E30, 0.0, 0.0 };
   double       avg       = 0.0;

   collList_foreach( samples, computeSample, &minMaxAvg );
   avg = minMaxAvg.sum / collList_size( samples );
   fprintf( out, "| %-10s | %9.2f | %9.2f | %9.2f |\n",
      attribute, minMaxAvg.min, minMaxAvg.max, avg );
   return true;
}

static bool saveSamplesToGnuPlot( collMapVoidPtrForeach * context ) {
   const char * attribute      = (const char *)context->key;
   collList     samples        = (collList    )context->value;
   char         filename[1024] = "";
   FILE *       out;
   unsigned int count;
   unsigned int index;

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

void Performance_saveToDisk( void ) {
   FILE * out;
   if( ! s_enabled ) {
      return;
   }

   out = fopen( "perfo.txt", "wt" );
   fprintf( out, "+------------+-----------+-----------+-----------+\n" );
   fprintf( out, "| Attribute  |    Min µs |    Max µs |    Avg µs |\n" );
   fprintf( out, "+------------+-----------+-----------+-----------+\n" );
   collMapVoidPtr_foreach( s_samplesByAttribute, saveSamplesToText, out );
   fprintf( out, "+------------+-----------+-----------+-----------+\n" );
   fclose( out );
   collMapVoidPtr_foreach( s_samplesByAttribute, saveSamplesToGnuPlot, out );
}
