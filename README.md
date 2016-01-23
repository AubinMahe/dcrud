DCRUD
=====

# Goal #

DCRUD manage distributed collections of items - local caches - providing CRUD interface (**C**reate, **R**ead, **U**pdate, **D**elete). All items dispose of an identity, unique system wide. Items may reference others ([UML association](https://en.wikipedia.org/wiki/Association_%28object-oriented_programming%29)). An item may be internally structured ([UML composition](https://en.wikipedia.org/wiki/Object_composition)), treated as a whole, indivisible piece of data. Granularity of a publication is the whole item, which size are expected to be small (<2 kB).

DCRUD offers remote asynchronous operations too, grouped into interfaces. If an operation have to return a result, a callback must be provided, see [IRequired](Java/src/org/hpms/mw/dcrud/IRequired.java), [ICallback](Java/src/org/hpms/mw/dcrud/ICallback.java), [IProvided](Java/src/org/hpms/mw/dcrud/IProvided.java), [IOperation](Java/src/org/hpms/mw/dcrud/IOperation.java), [Arguments](Java/src/org/hpms/mw/dcrud/Arguments.java).

# Features #

 - Data distribution using Publish/Subscribe paradigm on LAN using multicast, unicast UDP, TCP streams:
    - Each type is identified by a `ClassID`.
    - Factories are attached to types.
    - `Topic` allows a subscriber to declare interest to several instances of a type.
    - Each subscription is identified by a `Topic`.
    - There are no item without `Topic` but they may have no subscriber (no consumer) and at least one publisher.
 - Remote operation:
    - Asynchronous
    - Synchronous
    - Returned result (out parameters)
 - Multi-OS:
    - Linux
    - Windows
 - Multi-languages:
    - Java
    - C
    - C-static-allocation (*to be completed*)
    - C++
 - Low level resources (CPU, RAM) consumption.

# How to build #

Hand made Makefiles, Eclipse and Visual Studio project are provided. Use Eclipse JDT with CDT plugin or Visual Studio 2015 or type `make` to build `dcrud-c-lib`, `dcrud-c-test`, `dcrud-cpp-lib`, `dcrud-cpp-test`, `jar-lib`, `jar-test`. *TODO: use autoconf/automake (help welcome)*

# API Overview #

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

# Design #

- Interfaces and implementation are separated and implementation is hidden, even in C, thanks to [Abstract Data Type (ADT)](https://en.wikipedia.org/wiki/Abstract_data_type) intensive usage.
- Implementations are organized into several modules, i.e. `packages` in Java, prefixes (in lower case) in C, namespaces in C++:
   - `coll` : collections, *java.util.List,Map,Set* in Java, *std::list,map,set*, in C++
   - `dcrud`: API and implementation of DCRUD itself, *org.hpms.dcrud* in Java
   - `io`   : low level network communications, *java.nio* in Java, C++ implementation partially wraps C implementation (inline code only)*
   - `os`   : operating system abstraction, *doesn't exists in Java, C++ implementation partially wraps C implementation (inline code only)*
   - `util` : debugging, performances probes, system calls checking, errors reporting and logging, *doesn't exists in Java and C++*.
- Java implementation is pure, without any kind of JNI
- C++ implementation depends on C implementation because it uses `os` and `io` modules

## Implementation of inheritance in C language ##

Shared piece of data are derived from `Shareable`. dcrud use delegation to emulate this behavior in C language. Bidirectional link initialization between base and inherited instances are made by dcrud library, see [C API](http://aubinmahe.github.io/doxygen/html).

# Discovery #

In order to connect each participant to others a mechanism must be offered to register publishers - indexed and required by data or interface names - and connect them to subscribers.
A centralized registry, like CORBA Naming Service, will be a bottleneck and a single point of failure, when DCRUD offer a distributed registry based on dedicated protocol, over TCP.

## Notes: ##

An implementation of the registry based on constants is provided in the examples module to address statically designed systems. In this case no discovery occurs, the preceding protocol isn't deployed but the same API is used (see [IRegistry](Java/src/org/hpms/mw/dcrud/IRegistry.java)).

## Protocol ##

### At boot time: ###

Each participant declare a TCP server and a TCP client connected to a well known address.

```
  P1                           P2       P3
--+--                        --+--    --+--
  |                            |        |
  +--+                         |        |
  |  | Open TCP server         |        |
  |<-+                         |        |
  | Open socket                +--+     |
  |<---------------------------+  | Open TCP server
  | Send registry              |<-+     |
  |<---------------------------+        |
  +--+                         |        |
  |  | MD5 on current registry |        |
  |  | Merge registry, not replace it because local updates may occurs
  |<-+                         |        |
  | If modified and MD5 sums doesn't match, send MD5+registry
  +--------------------------->|        |
  |                            +--+     |
  |                            |  | MD5 on current registry
  |                            |  | Merge registry
  |                            |<-+     |
  |                            | Don't transmit registry because MD5 sums are equals
  | Open socket                |        +--+
  |<------------------------------------+  | Open TCP server
  | Send MD5+registry          |        |<-+
  |<------------------------------------+
  |--+                         |        |
  |  | Merge registry          |        |
  |<-+                         |        |
  | If modified and MD5 sums doesn't match, send MD5+registry
  +--------------------------->|        |
  +------------------------------------>|
  |                            +--+     |
  |                            |  | MD5 on current registry
  |                            |  | Merge registry
  |                            |<-+     |
  |                            | Don't transmit registry because MD5 sums are equals
  |                            |        +--+
  |                            |        |  | MD5 on current registry
  |                            |        |  | Merge registry
  |                            |        |<-+
  |                            |        | Don't transmit registry because MD5 sums are equals
```
At this point, all participants are server and client of each others, but none is THE server. Failure of one participant doesn't have any other impact on the whole network than the lack of its resources.

### New offer diffusion: ###
Each time a participant receives a Registry, it merges it to its own and broadcast it if an update has occurred. To minimize the number of transmissions, a MD5 sum is made with the previous registry and transmitted with the current registry. If a participant receives an update but have the same previous registry, it updates its own but it have'nt to transmit the updated registry because this has already be done by the sender.
```
  P1                           P2       P3
--+--                        --+--    --+--
  | Add interface "IShape"     |        |
  |<---------------------------+        |
  |--+                         |        |
  |  | Add "IShape;B" association to registry
  |<-+                         |        |
  |              Send registry |        |
  +--------------------------->|        |
  |                       Send registry |
  +------------------------------------>|
  | Add data "Shape"           |        |
  |<---------------------------+        |
  |--+                         |        |
  |  | Add "Shape;B" association to registry
  |<-+                         |        |
  |             Send registry  |        |
  +--------------------------->|        |
  |                       Send registry |
  +------------------------------------>|
```
### Test of distributed registry ##

See [tests.registry.GUI](Java/test/tests/registry/GUI.java) and [testRegistry.sh](Java/testRegistry.sh) for a test of the protocol with 12 participants.

### When a participant shut down:###

1. Gently: a message is send to one which update its registry and then broadcast it.
2. Abruptly (on failure): the first participant which detect that the resource are no longer available update its registry and then broadcast it.

# Persistence #

DCRUD isn't a database but since the data are in cache, it's possible to save each cache at periodic interval to disk to ease shutdown and restart without loosing data.

# Fault tolerance and Monitoring #

When persistence will be done, the participant's state will be saved periodically, so it may be restarted in case of failure thanks to a monitoring layer.

# Todo list (in priority order) #

 * Add registry mechanism
 * Complete Pub/Sub implementation using Topic - a channel between publisher and subscribers - in place of ClassID, which is only a key of factory
 * Add UDP/IP protocol
 * Add binary, XML and JSON persistence
 * Add monitoring layer to achieve fault tolerance
 * Add TCP/IP protocol
 * Update C static allocation implementation for embedded, low resource targets
 * Add TCP-IP streams services and protocol for big or continuous data
 * Build a code generator for C, C++ and Java languages to ease implementation of serialize methods and IOperation implementation. Generators will use XML representation based on a schema (XSD) designed to be as close as possible to UML concepts.
 * Add HTML5-WebSocket support
 * Add JavaScript support to address HTML5-WebSockets

# Interfaces sample usage #

Java: [ShapesUI](Java/test/tests/shapes/ShapesUI.java) has been *simplified* below for clarity reason.

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

C: [shapes_publisher.c](C/test/test_008.c) has been *simplified* below for clarity reason.

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
