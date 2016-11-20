#pragma once

#include <dcrud/ClassID.hpp>
#include <dcrud/GUID.hpp>

#include <map>
#include <string>
#include <stdexcept>

namespace dcrud {

   class Shareable;
   class ParticipantImpl;

   class Arguments {
   public:

      Arguments( void );

      ~ Arguments();

      void clear();

      bool isEmpty() const {
         return _args.empty();
      }

      size_t getCount() const {
         return _args.size();
      }

      void putNull     ( const std::string & key );
      void putByte     ( const std::string & key, byte                   value );
      void putBool     ( const std::string & key, bool                   value );
      void putShort    ( const std::string & key, short                  value );
      void putUShort   ( const std::string & key, unsigned short         value );
      void putInt      ( const std::string & key, int                    value );
      void putUInt     ( const std::string & key, unsigned int           value );
      void putLong     ( const std::string & key, const int64_t &        value );
      void putULong    ( const std::string & key, const uint64_t &       value );
      void putFloat    ( const std::string & key, float                  value );
      void putDouble   ( const std::string & key, const double &         value );
      void putString   ( const std::string & key, const std::string &    value );
      void putClassID  ( const std::string & key, const ClassID &        value );
      void putGUID     ( const std::string & key, const GUID &           value );
      void putShareable( const std::string & key, const Shareable *      value );

      bool isNull      ( const std::string & key ) const;

      bool getByte     ( const std::string & key, byte &           value ) const;
      bool getBool     ( const std::string & key, bool &           value ) const;
      bool getShort    ( const std::string & key, short &          value ) const;
      bool getUShort   ( const std::string & key, unsigned short & value ) const;
      bool getInt      ( const std::string & key, int &            value ) const;
      bool getUInt     ( const std::string & key, unsigned int &   value ) const;
      bool getLong     ( const std::string & key, int64_t &        value ) const;
      bool getULong    ( const std::string & key, uint64_t &       value ) const;
      bool getFloat    ( const std::string & key, float &          value ) const;
      bool getDouble   ( const std::string & key, double &         value ) const;
      bool getString   ( const std::string & key, std::string &    value ) const;
      bool getClassID  ( const std::string & key, ClassID &        value ) const;
      bool getGUID     ( const std::string & key, GUID &           value ) const;
      bool getShareable( const std::string & key, Shareable * &    value ) const;

      void setMode ( byte mode );

      void setQueue( byte queue );

      ClassID::Type getType( const std::string & key ) const;

      void serialize( io::ByteBuffer & message ) const;

   private:

      typedef std::map<std::string, void *> args_t;
      typedef args_t::iterator              argsIter_t;
      typedef args_t::const_iterator        argsCstIter_t;

      typedef std::map<std::string, ClassID::Type> types_t;
      typedef types_t::iterator                    typesIter_t;
      typedef types_t::const_iterator              typesCstIter_t;

      args_t  _args;
      types_t _types;
      byte    _mode;
      byte    _queue;
   };
}
