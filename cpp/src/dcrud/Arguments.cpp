#include <dcrud/Arguments.hpp>
#include <dcrud/IRequired.hpp>
#include <dcrud/ClassID.hpp>
#include <dcrud/Shareable.hpp>

using namespace dcrud;

Arguments::Arguments( void ) {
   setMode ( IRequired::ASYNCHRONOUS_DEFERRED );
   setQueue( IRequired::DEFAULT_QUEUE );
}

Arguments:: ~ Arguments() {
   for( argsIter_t it = _args.begin(); it != _args.end(); ++it ) {
      if( it->second ) {
         ClassID::Type type  = _types.find( it->first )->second;
         void *        value = it->second;
         switch( type ) {
         case ClassID::TYPE_NULL       : break;
         case ClassID::TYPE_BYTE       : delete (byte *          )value; break;
         case ClassID::TYPE_BOOLEAN    : delete (byte *          )value; break;
         case ClassID::TYPE_SHORT      : delete (unsigned short *)value; break;
         case ClassID::TYPE_INTEGER    : delete (unsigned int *  )value; break;
         case ClassID::TYPE_LONG       : delete (unsigned long * )value; break;
         case ClassID::TYPE_FLOAT      : delete (float *         )value; break;
         case ClassID::TYPE_DOUBLE     : delete (double *        )value; break;
         case ClassID::TYPE_STRING     : delete (std::string *   )value; break;
         case ClassID::TYPE_CLASS_ID   : delete (ClassID *       )value; break;
         case ClassID::TYPE_GUID       : delete (dcrud::GUID *          )value; break;
         case ClassID::TYPE_CALL_MODE  : delete (byte *          )value; break;
         case ClassID::TYPE_QUEUE_INDEX: delete (byte *          )value; break;
         case ClassID::TYPE_SHAREABLE  : break;
         case ClassID::LAST_TYPE       : break;
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

void Arguments::put( const std::string & key, const byte & value ) {
   _args [key] = new byte( value );
   _types[key] = ClassID::TYPE_BYTE;
}

void Arguments::put( const std::string & key, const bool & value ) {
   _args [key] = new byte( value ? 1 : 0 );
   _types[key] = ClassID::TYPE_BOOLEAN;
}

void Arguments::put( const std::string & key, const unsigned short & value ) {
   _args [key] = new unsigned short( value );
   _types[key] = ClassID::TYPE_SHORT;
}

void Arguments::put( const std::string & key, const unsigned int & value ) {
   _args [key] = new unsigned int( value );
   _types[key] = ClassID::TYPE_INTEGER;
}

void Arguments::put( const std::string & key, const uint64_t & value ) {
   _args [key] = new uint64_t( value );
   _types[key] = ClassID::TYPE_LONG;
}

void Arguments::put( const std::string & key, const float & value ) {
   _args [key] = new float( value );
   _types[key] = ClassID::TYPE_FLOAT;
}

void Arguments::put( const std::string & key, const double & value ) {
   _args [key] = new double( value );
   _types[key] = ClassID::TYPE_DOUBLE;
}

void Arguments::put( const std::string & key, const std::string & value ) {
   _args [key] = new std::string( value );
   _types[key] = ClassID::TYPE_STRING;
}

void Arguments::put( const std::string & key, const ClassID & value ) {
   _args [key] = new ClassID( value );
   _types[key] = ClassID::TYPE_CLASS_ID;
}

void Arguments::put( const std::string & key, const dcrud::GUID & value ) {
   _args [key] = new dcrud::GUID( value );
   _types[key] = ClassID::TYPE_GUID;
}

void Arguments::put( const std::string & key, const Shareable * value ) {
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

bool Arguments::get( const std::string & key, byte & value ) const {
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

bool Arguments::get( const std::string & key, bool & value ) const {
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

bool Arguments::get( const std::string & key, unsigned short & value ) const {
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

bool Arguments::get( const std::string & key, unsigned int & value ) const {
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

bool Arguments::get( const std::string & key, uint64_t & value ) const {
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

bool Arguments::get( const std::string & key, float & value ) const {
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

bool Arguments::get( const std::string & key, double & value ) const {
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

bool Arguments::get( const std::string & key, std::string & value ) const {
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

bool Arguments::get( const std::string & key, ClassID & value ) const {
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

bool Arguments::get( const std::string & key, dcrud::GUID & value ) const {
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

bool Arguments::get( const std::string & key, Shareable * & value ) const {
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
   _args ["@mode"] = new byte( mode );
   _types["@mode"] = ClassID::TYPE_CALL_MODE;
}

void Arguments::setQueue( byte queue ) {
   _args ["@queue"] = new byte( queue );
   _types["@queue"] = ClassID::TYPE_QUEUE_INDEX;
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
         case ClassID::TYPE_NULL       : /* No data. */                                   break;
         case ClassID::TYPE_BOOLEAN    : message.putBoolean( *(bool*            )value ); break;
         case ClassID::TYPE_BYTE       : message.putByte   ( *(byte *           )value ); break;
         case ClassID::TYPE_SHORT      : message.putShort  ( *(unsigned short * )value ); break;
         case ClassID::TYPE_INTEGER    : message.putInt    ( *(unsigned int *   )value ); break;
         case ClassID::TYPE_LONG       : message.putLong   ( *(uint64_t *       )value ); break;
         case ClassID::TYPE_FLOAT      : message.putFloat  ( *(float *          )value ); break;
         case ClassID::TYPE_DOUBLE     : message.putDouble ( *(double *         )value ); break;
         case ClassID::TYPE_STRING     : message.putString ( *(std::string *    )value ); break;
         case ClassID::TYPE_CLASS_ID   : ((ClassID *)value)->serialize( message );        break;
         case ClassID::TYPE_GUID       : ((dcrud::GUID *   )value)->serialize( message );        break;
         case ClassID::TYPE_CALL_MODE  : message.putByte   ( *(byte *           )value ); break;
         case ClassID::TYPE_QUEUE_INDEX: message.putByte   ( *(byte *           )value ); break;
         case ClassID::TYPE_SHAREABLE  : /* Already handled before this switch. */        break;
         default:
            throw std::invalid_argument( name );
         }
      }
   }
}
