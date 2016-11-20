#include <dcrud/Arguments.hpp>
#include <dcrud/IRequired.hpp>
#include <dcrud/ClassID.hpp>
#include <dcrud/Shareable.hpp>

using namespace dcrud;

Arguments::Arguments( void ) :
   _args (),
   _types(),
   _mode ( IRequired::ASYNCHRONOUS_DEFERRED ),
   _queue( IRequired::DEFAULT_QUEUE )
{}

Arguments:: ~ Arguments() {
   for( argsIter_t it = _args.begin(); it != _args.end(); ++it ) {
      if( it->second ) {
         ClassID::Type type  = _types.find( it->first )->second;
         void *        value = it->second;
         switch( type ) {
         case ClassID::TYPE_NULL            : break;
         case ClassID::TYPE_CHAR            :
         case ClassID::TYPE_BYTE            :
         case ClassID::TYPE_BOOLEAN         : delete (byte *          )value; break;
         case ClassID::TYPE_SHORT           : delete (short *         )value; break;
         case ClassID::TYPE_UNSIGNED_SHORT  : delete (unsigned short *)value; break;
         case ClassID::TYPE_INTEGER         : delete (int *           )value; break;
         case ClassID::TYPE_UNSIGNED_INTEGER: delete (unsigned int *  )value; break;
         case ClassID::TYPE_LONG            : delete (long *          )value; break;
         case ClassID::TYPE_UNSIGNED_LONG   : delete (unsigned long * )value; break;
         case ClassID::TYPE_FLOAT           : delete (float *         )value; break;
         case ClassID::TYPE_DOUBLE          : delete (double *        )value; break;
         case ClassID::TYPE_STRING          : delete (std::string *   )value; break;
         case ClassID::TYPE_CLASS_ID        : delete (ClassID *       )value; break;
         case ClassID::TYPE_GUID            : delete (dcrud::GUID *   )value; break;
         case ClassID::TYPE_SHAREABLE       : break;
         case ClassID::LAST_TYPE            : break;
         default:
            break;
         }
      }
   }
}

void Arguments::clear( void ) {
   _args .clear();
   _types.clear();
}

void Arguments::putNull( const std::string & key ) {
   _args [key] = 0;
   _types[key] = ClassID::TYPE_NULL;
}

void Arguments::putByte( const std::string & key, byte value ) {
   _args [key] = new byte( value );
   _types[key] = ClassID::TYPE_BYTE;
}

void Arguments::putBool( const std::string & key, bool value ) {
   _args [key] = new byte( value ? 1 : 0 );
   _types[key] = ClassID::TYPE_BOOLEAN;
}

void Arguments::putShort( const std::string & key, short value ) {
   _args [key] = new short( value );
   _types[key] = ClassID::TYPE_SHORT;
}

void Arguments::putUShort( const std::string & key, unsigned short value ) {
   _args [key] = new unsigned short( value );
   _types[key] = ClassID::TYPE_SHORT;
}

void Arguments::putInt( const std::string & key, int value ) {
   _args [key] = new int( value );
   _types[key] = ClassID::TYPE_INTEGER;
}

void Arguments::putUInt( const std::string & key, unsigned int value ) {
   _args [key] = new unsigned int( value );
   _types[key] = ClassID::TYPE_INTEGER;
}

void Arguments::putLong( const std::string & key, const int64_t & value ) {
   _args [key] = new int64_t( value );
   _types[key] = ClassID::TYPE_LONG;
}

void Arguments::putULong( const std::string & key, const uint64_t & value ) {
   _args [key] = new uint64_t( value );
   _types[key] = ClassID::TYPE_LONG;
}

void Arguments::putFloat( const std::string & key, float value ) {
   _args [key] = new float( value );
   _types[key] = ClassID::TYPE_FLOAT;
}

void Arguments::putDouble( const std::string & key, const double & value ) {
   _args [key] = new double( value );
   _types[key] = ClassID::TYPE_DOUBLE;
}

void Arguments::putString( const std::string & key, const std::string & value ) {
   _args [key] = new std::string( value );
   _types[key] = ClassID::TYPE_STRING;
}

void Arguments::putClassID( const std::string & key, const ClassID & value ) {
   _args [key] = new ClassID( value );
   _types[key] = ClassID::TYPE_CLASS_ID;
}

void Arguments::putGUID( const std::string & key, const dcrud::GUID & value ) {
   _args [key] = new dcrud::GUID( value );
   _types[key] = ClassID::TYPE_GUID;
}

void Arguments::putShareable( const std::string & key, const Shareable * value ) {
   _args [key] = const_cast<Shareable *>( value );
   _types[key] = ClassID::TYPE_SHAREABLE;
}

bool Arguments::isNull( const std::string & key ) const {
   typesCstIter_t it = _types.find( key );
   if( it == _types.end()) {
      return false;
   }
   return it->second;
}

bool Arguments::getByte( const std::string & key, byte & value ) const {
   argsCstIter_t it = _args.find( key );
   if( it == _args.end()) {
      return false;
   }
   const ClassID::Type & type = _types.find( it->first )->second;
   if( type != ClassID::TYPE_BYTE ) {
      return false;
   }
   value = *(const byte *)it->second;
   return true;
}

bool Arguments::getBool( const std::string & key, bool & value ) const {
   argsCstIter_t it = _args.find( key );
   if( it == _args.end()) {
      return false;
   }
   const ClassID::Type & type = _types.find( it->first )->second;
   if( type != ClassID::TYPE_BOOLEAN ) {
      return false;
   }
   value = (*(const byte *)it->second) != 0;
   return true;
}

bool Arguments::getShort( const std::string & key, short & value ) const {
   argsCstIter_t it = _args.find( key );
   if( it == _args.end()) {
      return false;
   }
   const ClassID::Type & type = _types.find( it->first )->second;
   if( type != ClassID::TYPE_SHORT ) {
      return false;
   }
   value = *(const short *)it->second;
   return true;
}

bool Arguments::getUShort( const std::string & key, unsigned short & value ) const {
   argsCstIter_t it = _args.find( key );
   if( it == _args.end()) {
      return false;
   }
   const ClassID::Type & type = _types.find( it->first )->second;
   if( type != ClassID::TYPE_SHORT ) {
      return false;
   }
   value = *(const unsigned short *)it->second;
   return true;
}

bool Arguments::getInt( const std::string & key, int & value ) const {
   argsCstIter_t it = _args.find( key );
   if( it == _args.end()) {
      return false;
   }
   const ClassID::Type & type = _types.find( it->first )->second;
   if( type != ClassID::TYPE_INTEGER ) {
      return false;
   }
   value = *(const int *)it->second;
   return true;
}

bool Arguments::getUInt( const std::string & key, unsigned int & value ) const {
   argsCstIter_t it = _args.find( key );
   if( it == _args.end()) {
      return false;
   }
   const ClassID::Type & type = _types.find( it->first )->second;
   if( type != ClassID::TYPE_INTEGER ) {
      return false;
   }
   value = *(const unsigned int *)it->second;
   return true;
}

bool Arguments::getLong( const std::string & key, int64_t & value ) const {
   argsCstIter_t it = _args.find( key );
   if( it == _args.end()) {
      return false;
   }
   const ClassID::Type & type = _types.find( it->first )->second;
   if( type != ClassID::TYPE_LONG ) {
      return false;
   }
   value = *(const int64_t *)it->second;
   return true;
}

bool Arguments::getULong( const std::string & key, uint64_t & value ) const {
   argsCstIter_t it = _args.find( key );
   if( it == _args.end()) {
      return false;
   }
   const ClassID::Type & type = _types.find( it->first )->second;
   if( type != ClassID::TYPE_LONG ) {
      return false;
   }
   value = *(const uint64_t *)it->second;
   return true;
}

bool Arguments::getFloat( const std::string & key, float & value ) const {
   argsCstIter_t it = _args.find( key );
   if( it == _args.end()) {
      return false;
   }
   const ClassID::Type & type = _types.find( it->first )->second;
   if( type != ClassID::TYPE_FLOAT ) {
      return false;
   }
   value = *(const float *)it->second;
   return true;
}

bool Arguments::getDouble( const std::string & key, double & value ) const {
   argsCstIter_t it = _args.find( key );
   if( it == _args.end()) {
      return false;
   }
   const ClassID::Type & type = _types.find( it->first )->second;
   if( type != ClassID::TYPE_DOUBLE ) {
      return false;
   }
   value = *(const double *)it->second;
   return true;
}

bool Arguments::getString( const std::string & key, std::string & value ) const {
   argsCstIter_t it = _args.find( key );
   if( it == _args.end()) {
      return false;
   }
   const ClassID::Type & type = _types.find( it->first )->second;
   if( type != ClassID::TYPE_STRING ) {
      return false;
   }
   value = *(const std::string *)it->second;
   return true;
}

bool Arguments::getClassID( const std::string & key, ClassID & value ) const {
   argsCstIter_t it = _args.find( key );
   if( it == _args.end()) {
      return false;
   }
   const ClassID::Type & type = _types.find( it->first )->second;
   if( type != ClassID::TYPE_CLASS_ID ) {
      return false;
   }
   value = *(const ClassID *)it->second;
   return true;
}

bool Arguments::getGUID( const std::string & key, dcrud::GUID & value ) const {
   argsCstIter_t it = _args.find( key );
   if( it == _args.end()) {
      return false;
   }
   const ClassID::Type & type = _types.find( it->first )->second;
   if( type != ClassID::TYPE_GUID ) {
      return false;
   }
   value = *(const dcrud::GUID *)it->second;
   return true;
}

bool Arguments::getShareable( const std::string & key, Shareable * & value ) const {
   argsCstIter_t it = _args.find( key );
   if( it == _args.end()) {
      return false;
   }
   const ClassID::Type & type = _types.find( it->first )->second;
   if( type != ClassID::TYPE_SHAREABLE ) {
      return false;
   }
   value = const_cast<Shareable *>((const Shareable *)it->second );
   return true;
}

void Arguments::setMode( byte mode ) {
   _mode = mode;
}

void Arguments::setQueue( byte queue ) {
   _queue = queue;
}

ClassID::Type Arguments::getType( const std::string & key ) const {
   typesCstIter_t it = _types.find( key );
   if( it == _types.end()) {
      return ClassID::LAST_TYPE;
   }
   return it->second;
}

void Arguments::serialize( io::ByteBuffer & message ) const {
   for( argsCstIter_t it = _args.begin(); it != _args.end(); ++it ) {
      const std::string & name  = it->first;
      const void *        value = it->second;
      const ClassID::Type type  = getType( name );
      message.putString( name );
      if( type == ClassID::TYPE_SHAREABLE ) {
         const Shareable * item = (const Shareable *)value;
         item->serializeClass( message );
         item->serialize( message );
      }
      else {
         ClassID::serialize( type, message );
         switch( type ) {
         case ClassID::TYPE_NULL            : /* No data. */ break;
         case ClassID::TYPE_BOOLEAN         : message.putBoolean( *(const bool*            )value ); break;
         case ClassID::TYPE_CHAR            :
         case ClassID::TYPE_BYTE            : message.putByte   ( *(const byte *           )value ); break;
         case ClassID::TYPE_SHORT           : message.putShort  ( *(const unsigned short * )value ); break;
         case ClassID::TYPE_UNSIGNED_SHORT  : message.putUShort ( *(const unsigned short * )value ); break;
         case ClassID::TYPE_INTEGER         : message.putInt    ( *(const unsigned int *   )value ); break;
         case ClassID::TYPE_UNSIGNED_INTEGER: message.putUInt   ( *(const unsigned int *   )value ); break;
         case ClassID::TYPE_LONG            : message.putLong   ( *(const uint64_t *       )value ); break;
         case ClassID::TYPE_UNSIGNED_LONG   : message.putULong  ( *(const uint64_t *       )value ); break;
         case ClassID::TYPE_FLOAT           : message.putFloat  ( *(const float *          )value ); break;
         case ClassID::TYPE_DOUBLE          : message.putDouble ( *(const double *         )value ); break;
         case ClassID::TYPE_STRING          : message.putString ( *(const std::string *    )value ); break;
         case ClassID::TYPE_CLASS_ID        : ((const ClassID *)value)->serialize( message );        break;
         case ClassID::TYPE_GUID            : ((const GUID *   )value)->serialize( message );        break;
         case ClassID::TYPE_SHAREABLE       : /* Already handled before this switch. */              break;
         case ClassID::LAST_TYPE:
         default:
            throw std::invalid_argument( name );
         }
      }
   }
}
