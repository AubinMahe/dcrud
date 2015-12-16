DCRUD
=====

Goal
----

DCRUD manage distributed collections of items - local caches - providing CRUD interface (**C**reate, **R**ead, **U**pdate, **D**elete). All items dispose of an identity, unique system wide. Items may reference others ([UML association](https://en.wikipedia.org/wiki/Association_%28object-oriented_programming%29)). An item may be internally structured ([UML composition](https://en.wikipedia.org/wiki/Object_composition)), treated as a whole, indivisible piece of data. Granularity of a publication is the whole item, which size are expected to be small (<2 kB).

DCRUD offers remote asynchronous operations too, grouped into interfaces. If an operation have to return a result, a callback must be provided, see [IRequired](Java/src/org/hpms/mw/distcrud/IRequired.java) and [IOperationInOut](Java/src/org/hpms/mw/distcrud/IOperationInOut.java).

Features
--------

 - Data distribution on LAN using multicast, the Internet (WAN) is not a target.
 - Remote operation, asynchronous or synchronous, with or without returned result.
 - Multi-OS: currently Windows and Linux
 - Multi-languages: Java, C, C-static-allocation, C++, and a lot of C friendly scripting languages because DCRUD is completely dynamic
 - Low level resources (CPU, RAM) consumption

Design and usage
----------------

Interfaces and implementation are separated and implementation is hidden, even in C, thanks to [Abstract Data Type (ADT)](https://en.wikipedia.org/wiki/Abstract_data_type) intensive usage.

Overview
--------

![UML diagram interfaces](interfaces.png "UML diagram interfaces")

*Interface and utility classes needed to use DCRUD*

- `Network` offers a single method to obtain a `IParticipant` instance for a particular ring or cloud, identified by its multicast address.
- `IParticipant` registers data factories, give the `IDispatcher` singleton and offers methods to create and get `ICache` instances which act as items caches.
- `ICache` provides **CRUD** interface plus `select`, `publish`, `refresh` to deal with several classes of `Shareable` item. It manages a local cache and tracks local and remote create, update, delete events to synchronize locals and remote caches using `refresh` and `publish` operations. `read` operation is always local. It's an applicative decision to use one or several caches.
- `Shareable` is an abstract class which owns a GUID (**G**lobal **U**nique **ID**entifier) and a `ClassID`.
- `Status` is a simple enumeration to report success or failure of the API implementation.
- `IDispatcher` allows application to declare theirs operation in terms of *provide* and *require*. Method `handleRequests` allow application to trigger the deferred operation requests when desired.
- `IRequired` is used by a client when `IProvided` is used to add `IOperation` which must be implemented by a server.
- `ICallback` may be implemented in case of asynchronous calls with returns results via `IRequired`.
- `SerializerHelper` (Java), `ioByteBuffer` (C), `io::ByteBuffer` (C++) is an utility class to deal with the network, handling endianness, serialization and deserialization.
- `Performance` is a simple timestamp recorder, GNU plot compatible, see [plot.sh](EclipseProject/plot.sh).

### Implementation of the inheritance in C language ###

Shared piece of data are derived from `Shareable`. dcrud use delegation to emulate this behavior in C language. Bidirectional link initialization between base and inherited instances are made by dcrud library, see [C API](http://aubinmahe.github.io/doxygen/html). 

Todo list (in priority order)
-----------------------------

 * Enhancing interfaces to support remote CRUD without knowledge of publisher
 * Adding hand made Makefiles to ease building without Eclipse
 * Adding UDP/IP, TCP/IP protocols
 * Building a code generator for C, C++ and Java languages to ease implementation of serialize methods and IOperation implementation
 * Adding TCP-IP streams services and protocol for big data
 * Adding HTML5-WebSocket support
 * Add JavaScript support to address HTML5-WebSockets
 * Updating C static allocation implementation for embedded, low resource targets

Interfaces sample usage
-----------------------

Java: [ShapesUI](Java/test/org/hpms/mw/distcrud/samples/shapes/ShapesUI.java) has been *simplified* below for clarity reason.

```Java
IParticipant participant = Network.join( new File( "network.xml" ), intrfc, name );
participant.registerClass( ShareableEllipse.CLASS_ID, ShareableEllipse::new );
participant.registerClass( ShareableRect   .CLASS_ID, ShareableRect   ::new );
cache = participant.createCache();
cache.subscribe( ShareableEllipse.CLASS_ID );
cache.subscribe( ShareableRect   .CLASS_ID );
IDispatcher dispatcher = participant.getDispatcher();
remoteShapesFactory = dispatcher.require( "IShapesFactory" );
Map<String, Object> arguments = new HashMap<>();
arguments.put( "class", ShareableEllipse.CLASS_ID );
arguments.put( "x"    , nextDouble(  0, 540 ));
arguments.put( "y"    , nextDouble(  0, 400 ));
arguments.put( "w"    , nextDouble( 40, 100 ));
arguments.put( "h"    , nextDouble( 20,  80 ));
remoteShapesFactory.call( "create", arguments );
cache.refresh();
```
------------

C: [shapes_publisher.c](C/test/shapes_publisher.c) has been *simplified* below for clarity reason.

```C
dcrudIRepositoryFactory repositories;
dcrudIRepository shapes;
ShareableShape * ellipse;

repositories = dcrudRepositoryFactoryBuilder_join( address, intrfc, port, 42 );
shapes = dcrudIRepositoryFactory_getRepository( repositories, shapeFactory );
ellipse = (ShareableShape *)malloc( sizeof( ShareableShape ));
memset( ellipse, 0 , sizeof( ShareableShape ));
dcrudShareable_init(
   ellipse,
   &ellipse->base,
   dcrudGUID_init( "shapes", 1 ),
   (dcrudShareable_setF        )ShareableShape_set,
   (dcrudShareable_serializeF  )ShareableShape_serialize,
   (dcrudShareable_unserializeF)ShareableShape_unserialize );
strcpy( ellipse->name, "Ellipse 001" );
ellipse->x = 640;
ellipse->y = 480;
ellipse->w = 64;
ellipse->h = 48;
dcrudIRepository_create( shapes, ellipse->base );
...
dcrudIRepository_publish( shapes );
nanosleep( &req, NULL );
move( ellipse );
dcrudIRepository_update( shapes, ellipse->base );
...
dcrudIRepository_publish( shapes );
nanosleep( &req, NULL );
move( ellipse );
dcrudIRepository_update( shapes, ellipse->base );
```
------------

C++: [shapes_publisher.cpp](cpp/test/shapes_publisher.cpp) has been *simplified* below for clarity reason.

```C++
struct ShapesSample {

   static const unsigned int LOOP_COUNT = 10000U;

   dcrud::IParticipant & _participant;
   dcrud::ICache &       _cache;
   dcrud::IDispatcher &  _dispatcher;

   ShapesSample( unsigned short publisherId, const char * intrfc ) :
      _participant( dcrud::Network::join( "network.cfg", intrfc, publisherId )),
      _cache      ( _participant.createCache()),
      _dispatcher ( _participant.getDispatcher())
   {
      ShareableShape::registerClasses   ( _participant );
      ShareableShape::registerOperations( _dispatcher  );
   }

   void run() {
      _cache.create( *new ShareableShape( ShareableShape::RectangleClassID ));
      _cache.create( *new ShareableShape( ShareableShape::EllipseClassID ));
      _cache.create( *new ShareableShape( ShareableShape::RectangleClassID ));
      _cache.create( *new ShareableShape( ShareableShape::EllipseClassID ));
      std::set<dcrud::Shareable *> snapshot;
      for( unsigned i = 0; i < LOOP_COUNT; ++i ) {
         _cache.publish();
         osSystem_sleep( 40U );
         _cache.values( snapshot );
         for( dcrud::shareablesIter_t it = snapshot.begin(); it != snapshot.end(); ++it ) {
            ShareableShape * shape = dynamic_cast<ShareableShape *>( *it );
            shape->move();
            _cache.update( *shape );
         }
         _dispatcher.handleRequests();
      }
   }
   
   ~ ShapesSample() {
      dcrud::shareables_t snapshot;
      _cache.values( snapshot );
      for( dcrud::shareablesIter_t it = snapshot.begin(); it != snapshot.end(); ++it ) {
         _cache.deleTe( **it );
      }
      _cache.publish();
      for( dcrud::shareablesIter_t it = snapshot.begin(); it != snapshot.end(); ++it ) {
         delete *it;
      }
      dcrud::Network::leave( _participant );
   }
};
```

API references
--------------

[Java API](http://aubinmahe.github.io/javadoc/)

[C API](http://aubinmahe.github.io/doxygen/html)

[C++ API](http://aubinmahe.github.io/doxygen-cpp/html)

How to build
------------

Use Eclipse JDT with CDT plugin to build `dcrud-c-lib`, `dcrud-c-test`, `dcrud-cpp-lib`, `dcrud-cpp-test`, then go to Java project and use ANT, target `jar-lib`. 
Jar `jar-lib` contains a tool which convert XML configuration file to a plain text for C and C++ implementations of DCRUD (to reduce dependencies).

*In progress*: hand made Makefiles are on the way...
