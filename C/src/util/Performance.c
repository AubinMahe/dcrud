#include <util/Performance.h>

#include <coll/List.h>
#include <coll/Map.h>

#include <util/String.h>

#include <stdio.h>
#include <string.h>

#ifdef _MSC_VER
#  pragma warning(disable : 4996)
#endif

static collMap s_samplesByAttribute = NULL;
static bool    s_enabled            = false;

utilStatus utilPerformance_enable( bool enabled ) {
   s_enabled = enabled;
   return UTIL_STATUS_NO_ERROR;
}

utilStatus utilPerformance_record( const char * attribute, uint64_t elapsed ) {
   collList   samples;
   uint64_t * sample;
   char *     key;
   utilStatus status;

   if( ! s_enabled ) {
      return UTIL_STATUS_NO_ERROR;
   }
   if( s_samplesByAttribute == NULL ) {
      status = collMap_new( &s_samplesByAttribute, (collComparator)collStringCompare );
      if( status != UTIL_STATUS_NO_ERROR ) {
         return status;
      }
   }
   status = collMap_get( s_samplesByAttribute, attribute, (collMapValue *)&samples );
   if( status == UTIL_STATUS_NOT_FOUND ) {
      status = collList_new( &samples );
      if( status != UTIL_STATUS_NO_ERROR ) {
         return status;
      }
      CHK(__FILE__,__LINE__,utilString_dup( &key, attribute ));
      CHK(__FILE__,__LINE__,collMap_put( s_samplesByAttribute, key, samples, NULL ))
      if( status != UTIL_STATUS_NO_ERROR ) {
         return status;
      }
   }
   else if( status != UTIL_STATUS_NO_ERROR ) {
      return status;
   }
   sample = (uint64_t *)malloc( sizeof( uint64_t ));
   if( sample == NULL ) {
      return UTIL_STATUS_TOO_MANY;
   }
   *sample = elapsed;
   status = collList_add( samples, sample );
   return status;
}

typedef struct MinMaxAvg_s {

   double min;
   double max;
   double sum;

} MinMaxAvg;

static utilStatus computeSample( collForeach * context ) {
   MinMaxAvg * minMaxAvg = (MinMaxAvg *)context->user;
   uint64_t *  sample    = (uint64_t * )context->value;

   if( *sample < minMaxAvg->min ) {
      minMaxAvg->min = (double)*sample;
   }
   if( *sample > minMaxAvg->max ) {
      minMaxAvg->max = (double)*sample;
   }
   minMaxAvg->sum += (double)*sample;
   return UTIL_STATUS_NO_ERROR;
}

static utilStatus saveSamplesToText( collForeach * context ) {
   FILE *       out       = (FILE *      )context->user;
   const char * attribute = (const char *)context->key;
   collList     samples   = (collList    )context->value;
   MinMaxAvg    minMaxAvg = { 1.0E30, 0.0, 0.0 };
   double       avg       = 0.0;
   unsigned     count     = 0U;

   collList_foreach( samples, computeSample, &minMaxAvg );
   collList_size( samples, &count );
   avg = minMaxAvg.sum / count;
   fprintf( out, "| %-10s | %9.2f | %9.2f | %9.2f |\n",
      attribute, minMaxAvg.min, minMaxAvg.max, avg );
   return UTIL_STATUS_NO_ERROR;
}

static utilStatus saveSamplesToGnuPlot( collForeach * context ) {
   const char * attribute      = (const char *)context->key;
   collList     samples        = (collList    )context->value;
   char         filename[1024] = "";
   FILE *       out;
   unsigned int count;
   unsigned int index;

   strcat( filename, attribute );
   strcat( filename, ".dat" );
   out   = fopen( filename, "wt" );
   collList_size( samples, &count );
   for( index = 0; index < count; ++index ) {
      uint64_t * measure = NULL;
      collList_get( samples, index, (collListItem*)&measure );
      double   value   = (double)*measure;
      fprintf( out, "%9.2f\n", value / 1000.0 );
   }
   return UTIL_STATUS_NO_ERROR;
}

utilStatus utilPerformance_saveToDisk( void ) {
   FILE * out;
   if( ! s_enabled ) {
      return UTIL_STATUS_NO_ERROR;
   }

   out = fopen( "perfo.txt", "wt" );
   if( out == NULL ) {
      return UTIL_STATUS_STD_API_ERROR;
   }
   fprintf( out, "+------------+-----------+-----------+-----------+\n" );
   fprintf( out, "| Attribute  |    Min µs |    Max µs |    Avg µs |\n" );
   fprintf( out, "+------------+-----------+-----------+-----------+\n" );
   collMap_foreach( s_samplesByAttribute, saveSamplesToText, out );
   fprintf( out, "+------------+-----------+-----------+-----------+\n" );
   fclose( out );
   return collMap_foreach( s_samplesByAttribute, saveSamplesToGnuPlot, out );
}
