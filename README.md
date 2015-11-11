DCRUD
=====

Goal
----

Manage distributed collections, providing CRUD interface (**C**reate,
**R**ead, **U**pdate, **D**elete).

Features
--------

 - Distributed on local area network, the Internet (WAN) is not a target
 - Multi-OS: currently Windows and Linux
 - Multi-languages: currently Java and C
 - Low level resources (CPU, RAM) consumption
 - Embeddable, static allocation

Design and usage
----------------

Interfaces and implementation are separated and implementation is hidden, even
in C, thanks to Abstract Data Type (ADT) intensive usage.

Overview
--------

![UML diagram interfaces](interfaces.png "UML diagram interfaces")

- `RepositoryFactoryBuilder` offers a single method to obtain a
  `IRepositoryFactory` instance for a particular ring or cloud, identified by
  its multicast address.
- `IRepositoryFactory` offers a single method to obtain a `IRepository`
  instance dedicated to a particular polymorphic class.
- `IRepository` provides **CRUD** interface plus `select`, `publish`,
  `refresh` to deal with one kind of `Shareable` item.
- `Shareable` is an abstract class which owns a GUID (**G**lobal **U**nique
  **ID**entifier)
- `SerializerHelper` is an utility static class which provides methods to
  deal with the network, handling endianness, serialization and
  deserialization.
  
**ShareableXXX** classes belongs to samples space.
  
### Implementation of the inheritance in C language ###

Shared piece of data are derived from `Shareable`. dcrud use delegation to
emulate this behavior in C language. Bidirectional link initialization between
base and inherited instances are made by dcrud library, see
[dcrudShareable_init()](dcrudShareable_init). 

Interfaces sample usage
-----------------------

Java: [ShapesUI](Java/test/org/hpms/mw/distcrud/sample2/ShapesUI.java) has been
*simplified* below for clarity reason.

```Java
IRepositoryFactory repositories =
   RepositoryFactoryBuilder.join( "224.0.0.3", "192.168.1.6", 2416 );
IRepository<ShareableShape> shapes;
   repositories.getRepository( 42, true, classId -> new ShareableEllipse());
ShareableEllipse ellipse =
   new ShareableEllipse( "Ellipse 001", new Ellipse( 640, 480, 64, 48 ));
shapes.create( ellipse );
shapes.publish();
...
ellipse.moveIt();
shapes.update( ellipse );
shapes.publish();
...
ellipse.moveIt();
shapes.update( ellipse );
shapes.publish();
```
------------

C: [tf_shapes_publisher](C/test/tf_shapes_publisher.c) has been *simplified*
below for clarity reason

```C
dcrudIRepositoryFactory repositories;
dcrudIRepository shapes;
ShareableShape * ellipse;
struct timespec req = { 0, 40*1000*1000 };

repositories = dcrudRepositoryFactoryBuilder_join( address, intrfc, port );
shapes = dcrudIRepositoryFactory_getRepository( repositories, 42, true, shapeFactory );
ellipse = (ShareableShape *)malloc( sizeof( ShareableShape ));
memset( ellipse, 0 , sizeof( ShareableShape ));
dcrudShareable_init(
   ellipse,
   &ellipse->base,
   classId,
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

Interfaces references
---------------------

### RepositoryFactoryBuilder ###

[RepositoryFactoryBuilder.java](Java/src/org/hpms/mw/distcrud/RepositoryFactoryBuilder),
[RepositoryFactoryBuilder.h](C/inc/dcrud/RepositoryFactoryBuilder.h)

Java:

```Java
IRepositoryFactory join( String address, String intrfc, int port )
```

C:

```C
dcrudIRepositoryFactory * dcrudRepositoryFactoryBuilder_join(
   const char *   address,
   const char *   intrfc,
   unsigned short port    )
```

  - `address` the multicast address to join
  - `intrfc` the network adapter to use (i.e. eth*x* in Java, IP address in C)
  - `port` the network port to listen or use to send datagram packets

### IRepositoryFactory ###

[IRepositoryFactory.java](src/org/hpms/mw/distcrud/IRepositoryFactory.java),
[IRepositoryFactory.h](C/inc/dcrud/IRepositoryFactory.h)

Java:

```Java
IRepository<T> getRepository(
      int                                    classId,
      boolean                                owner,
      Function<Integer, ? extends Shareable> factory )
```

C:

```C
typedef dcrudShareable * ( * dcrudShareableFactory)( int classId );

dcrudIRepository * dcrudIRepositoryFactory_getRepository(
   dcrudIRepositoryFactory * This,
   int                       classId,
   int                       owner,
   dcrudShareableFactory     factory )
```

  - `This`    is the instance of repository factory
  - `classId` is the class identifier
  - `owner`   is true when the caller own the instance, as a publisher
  - `factory` is the item factory

### IRepository ##

[IRepository.java](Java/src/org/hpms/mw/distcrud/IRepository.java),
[IRepository.h](C/inc/dcrud/IRepository.h)

#### create ####

Java:

```Java
void create( T item )
```

C:

```C
void dcrudIRepository_create( dcrudIRepository This, dcrudShareable item )
```

  - `This`    is the instance of repository factory
  - `item` is an instance of the class handled by this repository

#### read ####

Java:

```Java
T read( GUID id )
```

C:

```C
dcrudShareable * dcrudIRepository_read( dcrudIRepository This, dcrudGUID id )
```

  - `This` is the instance of repository factory
  - `id`   the id of the piece of data to read

#### update ####

Java:

```Java
void update( T item )
```

C:

```C
bool dcrudIRepository_update( dcrudIRepository This, dcrudShareable item )
```

  - `This`    is the instance of repository factory
  - `item` is an instance of the class handled by this repository

#### delete ####

Java:

```Java
void delete ( T item )
```

#### select ####

Java:

```Java
Map<GUID, T> select ( Predicate<T> query )
```

#### publish ####

Java:

```Java
void publish()
```

#### refresh ####

Java:

```Java
void refresh()
```

### GUID ###

### Shareable ###

<a name="dcrudShareable_init"></a>C'est là.

###SerializerHelper###
