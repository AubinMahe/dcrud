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
in C, thanks to of Abstract Data Type (ADT) intensive usage.

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
ellipse->base = dcrudShareable_init(
   ellipse,
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

Java, `org.hpms.mw.distcrud.RepositoryFactoryBuilder`:

```Java
IRepositoryFactory join( String address, String intrfc, int port )
```

C, `inc/dcrud/RepositoryFactoryBuilder.h`:

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

Java, `org.hpms.mw.distcrud.IRepositoryFactory`:

```Java
IRepository<T> getRepository(
      int                                    classId,
      boolean                                owner,
      Function<Integer, ? extends Shareable> factory )
```

C, `inc/dcrud/IRepositoryFactory.h`:

```C
typedef dcrudShareable * ( * dcrudShareableFactory)( int classId );

CREATE_ADT( dcrudIRepositoryFactory );

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

###IRepository##

###Shareable###

###SerializerHelper###
