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
 - Multi-languages: currently Java and C, C++ may be addressed in a near future.
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
- `ICallback` must be implemented in case of asynchronous calls via `IRequired`.
- `SerializerHelper` is an utility static class which provides methods to deal with the network, handling endianness, serialization and deserialization.
- `Performance` is a simple timestamp recorder, GNU plot compatible, see [plot.sh](EclipseProject/plot.sh).

### Implementation of the inheritance in C language ###

Shared piece of data are derived from `Shareable`. dcrud use delegation to emulate this behavior in C language. Bidirectional link initialization between base and inherited instances are made by dcrud library, see [dcrudShareable_init()](#dcrudShareable_init). 

Todo list (in priority order)
-----------------------------

 * Add UDP/IP and TCP/IP support
 * Add streams services for big data
 * Update C static allocation implementation

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
// update JavaFX graphics
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

API references
--------------

[Java API](http://aubinmahe.github.io/javadoc/)

[C API](http://aubinmahe.github.io/doxygen/html)

