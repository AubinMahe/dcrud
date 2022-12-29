#include <util/CmdLine.h>
#include <util/Pool.h>
#include "magic.h"
#include "poolSizes.h"

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef _MSC_VER
#  define strcasecmp stricmp
#else
#  include <strings.h>
#endif

#define utilCmdLine_KEY_LEN     40
#define utilCmdLine_VALUE_LEN  100
#define utilCmdLine_MAX_ARGS    16

typedef enum {

   CmdLineType_BOOLEAN,
   CmdLineType_CHAR,
   CmdLineType_BYTE,
   CmdLineType_SHORT,
   CmdLineType_USHORT,
   CmdLineType_INT,
   CmdLineType_UINT,
   CmdLineType_FLOAT,
   CmdLineType_DOUBLE,
   CmdLineType_STRING

} CmdLineType_t;

typedef union CmdLineValue_u {

   bool           t; /* t like 'test' */
   char           c;
   byte           b;
   short          s;
   unsigned short us;
   int            i;
   unsigned int   ui;
   float          f;
   double         d;
   char           sz[utilCmdLine_VALUE_LEN];

} CmdLineValue_t;

typedef struct CmdLineDef_s {

   char           key [utilCmdLine_KEY_LEN];
   size_t         len;
   CmdLineType_t  type;
   CmdLineValue_t dflt;
   CmdLineValue_t value;

} CmdLineDef;

typedef struct utilCmdLineImpl_s {

   unsigned int magic;
   CmdLineDef   defs[utilCmdLine_MAX_ARGS];
   unsigned int count;

} utilCmdLineImpl;

UTIL_DEFINE_SAFE_CAST( utilCmdLine     )
UTIL_POOL_DECLARE    ( utilCmdLineImpl )

static utilStatus addValue(
   utilCmdLineImpl *      This,
   const char *           key,
   CmdLineType_t          type,
   const CmdLineValue_t * value )
{
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( This->count < utilCmdLine_MAX_ARGS ) {
      This->defs[This->count].key[0] = '\0';
      strcat( This->defs[This->count].key, "--" );
      strcat( This->defs[This->count].key, key );
      strcat( This->defs[This->count].key, "=" );
      This->defs[This->count].len   = (size_t)strlen( This->defs[This->count].key );
      This->defs[This->count].dflt  = *value;
      This->defs[This->count].type  = type;
      This->defs[This->count].value = *value;
      This->count++;
   }
   else {
      status = UTIL_STATUS_TOO_MANY;
   }
   return status;
}

static utilStatus getValue(
   const utilCmdLine       self,
   const char *            key,
   CmdLineType_t           type,
   const CmdLineValue_t ** value )
{
   utilStatus status = UTIL_STATUS_NO_ERROR;
   utilCmdLineImpl * This = utilCmdLine_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      unsigned i;
      status = UTIL_STATUS_NOT_FOUND;
      for( i = 0; ( i < This->count ) && ( status == UTIL_STATUS_NOT_FOUND ); ++i ) {
         CmdLineDef * def       = This->defs+i;
         const char * candidate = def->key + 2;
         size_t       len       = def->len - 3;
         if( 0 == strncmp( candidate, key, len )) {
            if( def->type == type ) {
               *value = &( def->value );
               status = UTIL_STATUS_NO_ERROR;
            }
            else {
               status = UTIL_STATUS_TYPE_MISMATCH;
            }
         }
      }
   }
   return status;
}

utilStatus utilCmdLine_new( utilCmdLine * self ) {
   utilCmdLineImpl * This = NULL;
   utilStatus status = UTIL_STATUS_NO_ERROR;
   UTIL_ALLOCATE_ADT( utilCmdLine, self, This );
   return status;
}

utilStatus utilCmdLine_delete( utilCmdLine * self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( NULL == self ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      utilCmdLine_safeCast( *self, &status );
      if( status == UTIL_STATUS_NO_ERROR ) {
         UTIL_RELEASE( utilCmdLineImpl );
      }
   }
   return status;
}

utilStatus utilCmdLine_addBoolean( utilCmdLine self, const char * key, bool dflt ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   utilCmdLineImpl * This = utilCmdLine_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      CmdLineValue_t value;
      memset( &value, 0, sizeof( value ));
      value.t = dflt;
      status = addValue( This, key, CmdLineType_BOOLEAN, &value );
   }
   return status;
}

utilStatus utilCmdLine_addChar( utilCmdLine self, const char * key, char dflt ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   utilCmdLineImpl * This = utilCmdLine_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      CmdLineValue_t value;
      memset( &value, 0, sizeof( value ));
      value.c = dflt;
      status = addValue( This, key, CmdLineType_CHAR, &value );
   }
   return status;
}

utilStatus utilCmdLine_addByte( utilCmdLine self, const char * key, byte dflt ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   utilCmdLineImpl * This = utilCmdLine_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      CmdLineValue_t value;
      memset( &value, 0, sizeof( value ));
      value.b = dflt;
      status = addValue( This, key, CmdLineType_BYTE, &value );
   }
   return status;
}

utilStatus utilCmdLine_addShort( utilCmdLine self, const char * key, short dflt ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   utilCmdLineImpl * This = utilCmdLine_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      CmdLineValue_t value;
      memset( &value, 0, sizeof( value ));
      value.s = dflt;
      status = addValue( This, key, CmdLineType_SHORT, &value );
   }
   return status;
}

utilStatus utilCmdLine_addUShort( utilCmdLine self, const char * key, unsigned short dflt ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   utilCmdLineImpl * This = utilCmdLine_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      CmdLineValue_t value;
      memset( &value, 0, sizeof( value ));
      value.us = dflt;
      status = addValue( This, key, CmdLineType_USHORT, &value );
   }
   return status;
}

utilStatus utilCmdLine_addInt( utilCmdLine self, const char * key, int dflt ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   utilCmdLineImpl * This = utilCmdLine_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      CmdLineValue_t value;
      memset( &value, 0, sizeof( value ));
      value.i = dflt;
      status = addValue( This, key, CmdLineType_INT, &value );
   }
   return status;
}

utilStatus utilCmdLine_addUInt( utilCmdLine self, const char * key, unsigned int dflt ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   utilCmdLineImpl * This = utilCmdLine_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      CmdLineValue_t value;
      memset( &value, 0, sizeof( value ));
      value.ui = dflt;
      status = addValue( This, key, CmdLineType_UINT, &value );
   }
   return status;
}

utilStatus utilCmdLine_addFloat( utilCmdLine self, const char * key, float dflt ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   utilCmdLineImpl * This = utilCmdLine_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      CmdLineValue_t value;
      memset( &value, 0, sizeof( value ));
      value.f = dflt;
      status = addValue( This, key, CmdLineType_FLOAT, &value );
   }
   return status;
}

utilStatus utilCmdLine_addDouble( utilCmdLine self, const char * key, double dflt ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   utilCmdLineImpl * This = utilCmdLine_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      CmdLineValue_t value;
      memset( &value, 0, sizeof( value ));
      value.d = dflt;
      status = addValue( This, key, CmdLineType_DOUBLE, &value );
   }
   return status;
}

utilStatus utilCmdLine_addString( utilCmdLine self, const char * key, const char * dflt ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   utilCmdLineImpl * This = utilCmdLine_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      CmdLineValue_t value;
      memset( &value, 0, sizeof( value ));
      strncpy( value.sz, dflt, utilCmdLine_VALUE_LEN );
      value.sz[utilCmdLine_VALUE_LEN-1] = '\0';
      status = addValue( This, key, CmdLineType_STRING, &value );
   }
   return status;
}

static int parseInt( const char * value, int min, int max, utilStatus * status ) {
   char * err = NULL;
   int    base;
   int    i;

   if( 0 == strncmp( value, "0x", 2 )) {
      value += 2;
      base = 16;
   }
   else {
      base = 10;
   }
   i = (int)strtol( value, &err, base );
   if( err && *err == '\0' ) {
      if( min <= i && i <= max ) {
         *status = UTIL_STATUS_NO_ERROR;
         return i;
      }
      *status = UTIL_STATUS_OUT_OF_RANGE;
   }
   else {
      *status = UTIL_STATUS_TYPE_MISMATCH;
   }
   return 0;
}

static unsigned long int parseUInt( const char * value, utilStatus * status ) {
   char * err = NULL;
   int base;
   unsigned long int u;

   if( 0 == strncmp( value, "0x", 2 )) {
      value += 2;
      base = 16;
   }
   else {
      base = 10;
   }
   u = strtoul( value, &err, base );
   if( err && *err == '\0' ) {
      *status = UTIL_STATUS_NO_ERROR;
      return u;
   }

   *status = UTIL_STATUS_TYPE_MISMATCH;
   return 0U;
}

static float parseFloat( const char * value, utilStatus * status ) {
   char * err = NULL;
   float  f   = strtof( value, &err );

   if( err && *err == '\0' ) {
      *status = UTIL_STATUS_NO_ERROR;
      return f;
   }
   *status = UTIL_STATUS_TYPE_MISMATCH;
   return 0.0f;
}

static double parseDouble( const char * value, utilStatus * status ) {
   char * err = NULL;
   double d   = strtod( value, &err );

   if( err && *err == '\0' ) {
      *status = UTIL_STATUS_NO_ERROR;
      return d;
   }
   *status = UTIL_STATUS_TYPE_MISMATCH;
   return 0.0;
}

utilStatus utilCmdLine_parse( utilCmdLine self, int argc, char * argv[] ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   utilCmdLineImpl * This = utilCmdLine_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      int i;
      for( i = 1; i < argc; ++i ) {
         bool notFound = true;
         unsigned j;
         for( j = 0; notFound && j < This->count; ++j ) {
            CmdLineDef * def = This->defs + j;
            if( 0 == strncmp( argv[i], def->key, def->len )) {
               const char * value = argv[i] + def->len;

               notFound = false;
               switch( def->type ) {
               case CmdLineType_BOOLEAN:
                  if( 0 == strcasecmp( value, "T" )) {
                     def->value.t = true;
                  }
                  else if( 0 == strcasecmp( value, "true" )) {
                     def->value.t = true;
                  }
                  else if( 0 == strcasecmp( value, "1" )) {
                     def->value.t = true;
                  }
                  else if( 0 == strcasecmp( value, "F" )) {
                     def->value.t = false;
                  }
                  else if( 0 == strcasecmp( value, "false" )) {
                     def->value.t = false;
                  }
                  else if( 0 == strcasecmp( value, "0" )) {
                     def->value.t = false;
                  }
                  else {
                     status = UTIL_STATUS_TYPE_MISMATCH;
                  }
               break;
               case CmdLineType_CHAR:
                  if( strlen( value ) > 1 ) {
                     status = UTIL_STATUS_TYPE_MISMATCH;
                  }
                  else {
                     def->value.b = (byte)value[0];
                  }
               break;
               case CmdLineType_BYTE:
                  def->value.b = (byte)parseInt( value, 0, 0xFF, &status );
               break;
               case CmdLineType_SHORT:
                  def->value.s = (short)parseInt( value, -0x8000, 0x7FFF, &status );
               break;
               case CmdLineType_USHORT:
                  def->value.s = (short)parseInt( value, 0, 0xFFFF, &status );
               break;
               case CmdLineType_INT:
                  def->value.i = parseInt( value, INT_MIN, INT_MAX, &status );
               break;
               case CmdLineType_UINT:
                  def->value.i = (int)parseUInt( value, &status );
               break;
               case CmdLineType_FLOAT:
                  def->value.f = parseFloat( value, &status );
               break;
               case CmdLineType_DOUBLE:
                  def->value.d = parseDouble( value, &status );
               break;
               case CmdLineType_STRING:
                  if( strlen( value ) >= utilCmdLine_VALUE_LEN ) {
                     status = UTIL_STATUS_OUT_OF_RANGE;
                  }
                  else {
                     strncpy( def->value.sz, value, utilCmdLine_VALUE_LEN );
                     def->value.sz[utilCmdLine_VALUE_LEN-1] = '\0';
                  }
               break;
               }
            }
         }
         if( notFound ) {
            fprintf( stderr, "Ignored argument: %s\n", argv[i] );
            status = UTIL_STATUS_NOT_FOUND;
         }
      }
   }
   return status;
}

utilStatus utilCmdLine_getBoolean( const utilCmdLine self, const char * key, bool * result ) {
   const CmdLineValue_t * value;
   utilStatus status = getValue( self, key, CmdLineType_BOOLEAN, &value );
   if( status == UTIL_STATUS_NO_ERROR ) {
      *result = ( value->t ? true : false );
   }
   return status;
}

utilStatus utilCmdLine_getShort( const utilCmdLine self, const char * key, short * result ) {
   const CmdLineValue_t * value;
   utilStatus status = getValue( self, key, CmdLineType_SHORT, &value );
   if( status == UTIL_STATUS_NO_ERROR ) {
      *result = value->s;
   }
   return status;
}

utilStatus utilCmdLine_getUShort( const utilCmdLine self, const char * key, unsigned short * result ) {
   const CmdLineValue_t * value;
   utilStatus status = getValue( self, key, CmdLineType_USHORT, &value );
   if( status == UTIL_STATUS_NO_ERROR ) {
      *result = value->us;
   }
   return status;
}

utilStatus utilCmdLine_getInt( const utilCmdLine self, const char * key, int * result ) {
   const CmdLineValue_t * value;
   utilStatus status = getValue( self, key, CmdLineType_INT, &value );
   if( status == UTIL_STATUS_NO_ERROR ) {
      *result = value->i;
   }
   return status;
}

utilStatus utilCmdLine_getUInt( const utilCmdLine self, const char * key, unsigned int * result ) {
   const CmdLineValue_t * value;
   utilStatus status = getValue( self, key, CmdLineType_UINT, &value );
   if( status == UTIL_STATUS_NO_ERROR ) {
      *result = value->ui;
   }
   return status;
}

utilStatus utilCmdLine_getFloat( const utilCmdLine self, const char * key, float * result ) {
   const CmdLineValue_t * value;
   utilStatus status = getValue( self, key, CmdLineType_FLOAT, &value );
   if( status == UTIL_STATUS_NO_ERROR ) {
      *result = value->f;
   }
   return status;
}

utilStatus utilCmdLine_getDouble( const utilCmdLine self, const char * key, double * result ) {
   const CmdLineValue_t * value;
   utilStatus status = getValue( self, key, CmdLineType_DOUBLE, &value );
   if( status == UTIL_STATUS_NO_ERROR ) {
      *result = value->d;
   }
   return status;
}

utilStatus utilCmdLine_getString( const utilCmdLine self, const char * key, const char ** result ) {
   const CmdLineValue_t * value;
   utilStatus status = getValue( self, key, CmdLineType_STRING, &value );
   if( status == UTIL_STATUS_NO_ERROR ) {
      *result = value->sz;
   }
   return status;
}
