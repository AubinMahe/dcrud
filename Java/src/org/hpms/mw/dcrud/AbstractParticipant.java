package org.hpms.mw.dcrud;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;
import java.util.TreeMap;
import java.util.function.Supplier;

abstract class AbstractParticipant implements IParticipant {

   protected final int _publisherId;
   protected final Cache[]                           _caches          = new Cache[256];
   protected final Dispatcher                        _dispatcher      = new Dispatcher( this );
   protected final Map<ClassID, Supplier<Shareable>> _localFactories  = new TreeMap<>();
   protected final Map<ClassID, ICRUD >              _remoteFactories = new TreeMap<>();
   protected final Map<Integer, ICallback>           _callbacks       = new HashMap<>();
   protected /* */ byte                              _cacheCount      = 0;
   protected /* */ int                               _callId          = 1;

   AbstractParticipant( int publisherId ) {
      _publisherId = publisherId;
   }

   Shareable newInstance( ClassID classId, ByteBuffer frame ) {
      synchronized( _localFactories ) {
         final Supplier<Shareable> factory = _localFactories.get( classId );
         if( factory == null ) {
            return null;
         }
         final Shareable item = factory.get();
         item.unserialize( frame );
         return item;
      }
   }

   @Override
   public void registerLocalFactory( ClassID id, Supplier<Shareable> factory ) {
      synchronized( _localFactories ) {
         _localFactories.put( id, factory );
      }
   }

   @Override
   public void registerRemoteFactory( ClassID id, ICRUD publisher ) {
      synchronized( _remoteFactories ) {
         _remoteFactories.put( id, publisher );
      }
   }

   @Override
   public ICache getDefaultCache() {
      synchronized( _caches ) {
         return _caches[0];
      }
   }

   @Override
   public ICache createCache() {
      synchronized( _caches ) {
         return _caches[_cacheCount++] = new Cache( this );
      }
   }

   @Override
   public ICache getCache( byte cacheIndex ) {
      synchronized( _caches ) {
         return _caches[cacheIndex];
      }
   }

   @Override
   public IDispatcher getDispatcher() {
      return _dispatcher;
   }

   int getPublisherId() {
      return _publisherId;
   }

   abstract void pushCreateOrUpdateItem( Shareable item ) throws IOException;

   void publishUpdated( Set<? extends Shareable> updated ) throws IOException {
      for( final Shareable item : updated ) {
         pushCreateOrUpdateItem( item );
      }
   }

   abstract void pushDeleteItem( Shareable item ) throws IOException;

   void publishDeleted( Set<? extends Shareable> deleted ) throws IOException {
      for( final Shareable item : deleted ) {
         pushDeleteItem( item );
      }
   }

   abstract void call( String intrfcName, String opName, Arguments args, int callId ) throws IOException;

   void call( String intrfcName, String opName, Arguments args, ICallback callback ) throws IOException {
      assert callback != null;
      call( intrfcName, opName, args, _callId );
      _callbacks.put( _callId++, callback );
   }

   void dataDelete( GUID id ) {
      synchronized( _caches ) {
         for( int i = 0; i < _cacheCount; ++i ) {
            _caches[i].deleteFromNetwork( id );
         }
      }
   }

   void dataUpdate( ByteBuffer frame, int payloadSize ) {
      synchronized( _caches ) {
         for( int i = 0; i < _cacheCount; ++i ) {
            final byte[] copy = new byte[payloadSize];
            System.arraycopy( frame.array(), frame.position(), copy, 0, payloadSize );
            final ByteBuffer fragment = ByteBuffer.wrap( copy );
//            Dump.dump( fragment );
            _caches[i].updateFromNetwork( fragment );
         }
      }
   }

   void callback( String intrfcName, String opName, Arguments args, int callId ) {
      final ICallback callback = _callbacks.get( callId );
      if( callback == null ) {
         System.err.printf( "Unknown Callback received: %s.%s, id: %d\n",
            intrfcName, opName, -callId );
      }
      else {
         callback.callback( intrfcName, opName, args );
      }
   }

   public boolean create( ClassID clsId, Arguments how ) throws IOException {
      if( clsId == null ) {
         return false;
      }
      final ICRUD publisher = _remoteFactories.get( clsId );
      if( publisher == null ) {
         return false;
      }
      publisher.create( how );
      return true;
   }

   public boolean update( GUID id, Arguments how ) throws IOException {
      for( int i = 0; i < _cacheCount; ++i ) {
         final Shareable item = _caches[i].read( id );
         if( item != null ) {
            final ICRUD publisher = _remoteFactories.get( item._class );
            if( publisher == null ) {
               return false;
            }
            publisher.update( item, how );
            return true;
         }
      }
      return false;
   }

   public boolean delete( GUID id ) throws IOException {
      for( int i = 0; i < _cacheCount; ++i ) {
         final Shareable item = _caches[i].read( id );
         if( item != null ) {
            final ICRUD publisher = _remoteFactories.get( item._class );
            if( publisher == null ) {
               return false;
            }
            publisher.delete( item );
            return true;
         }
      }
      return false;
   }
}
