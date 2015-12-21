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

      void putNull( const std::string & key );
      void put    ( const std::string & key, const byte &           value );
      void put    ( const std::string & key, const bool &           value );
      void put    ( const std::string & key, const unsigned short & value );
      void put    ( const std::string & key, const unsigned int &   value );
      void put    ( const std::string & key, const uint64_t &       value );
      void put    ( const std::string & key, const float &          value );
      void put    ( const std::string & key, const double &         value );
      void put    ( const std::string & key, const std::string &    value );
      void put    ( const std::string & key, const ClassID &        value );
      void put    ( const std::string & key, const GUID &           value );
      void put    ( const std::string & key, const Shareable *      value );

      bool isNull ( const std::string & key ) const;
      bool get    ( const std::string & key, byte &           value ) const;
      bool get    ( const std::string & key, bool &           value ) const;
      bool get    ( const std::string & key, unsigned short & value ) const;
      bool get    ( const std::string & key, unsigned int &   value ) const;
      bool get    ( const std::string & key, uint64_t &       value ) const;
      bool get    ( const std::string & key, float &          value ) const;
      bool get    ( const std::string & key, double &         value ) const;
      bool get    ( const std::string & key, std::string &    value ) const;
      bool get    ( const std::string & key, ClassID &        value ) const;
      bool get    ( const std::string & key, GUID &           value ) const;
      bool get    ( const std::string & key, Shareable * &    value ) const;

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
   };
}
