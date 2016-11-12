CC = gcc
AR = ar
UNAME_S := $(shell uname -s)
CFLAGS_COMMON  = -I inc -I src -fPIC
ifeq ($(UNAME_S),Linux)
   CFLAGS_COMMON += -std=gnu11 -pthread -D_FORTIFY_SOURCE=2
endif
CFLAGS_DEBUG   = $(CFLAGS_COMMON) -O0 -g3 -Wno-long-long -pedantic -W -Wall
CFLAGS_RELEASE = $(CFLAGS_COMMON) -O3
VALGRIND = valgrind --leak-check=full --show-leak-kinds=all
LIB_SRCS =\
 src/util/CmdLine.c\
 src/util/Dump.c\
 src/util/magic.c\
 src/util/Performance.c\
 src/util/Pool.c\
 src/util/String.c\
 src/util/Status.c\
 src/util/Trace.c\
\
 src/os/magic.c\
 src/os/Mutex.c\
 src/os/System.c\
 src/os/threads.c\
\
 src/io/ByteBuffer.c\
 src/io/InetSocketAddress.c\
 src/io/magic.c\
\
 src/coll/Comparator.c\
 src/coll/List.c\
 src/coll/magic.c\
 src/coll/Map.c\
 src/coll/Set.c\
\
 src/dcrud/Arguments.c\
 src/dcrud/Cache.c\
 src/dcrud/Callback.c\
 src/dcrud/ClassID.c\
 src/dcrud/DebugSettings.c\
 src/dcrud/Dispatcher.c\
 src/dcrud/GUID.c\
 src/dcrud/IRegistry.c\
 src/dcrud/magic.c\
 src/dcrud/Network.c\
 src/dcrud/NetworkReceiver.c\
 src/dcrud/ParticipantImpl.c\
 src/dcrud/Shareable.c\
\
 src/channel/CoDec.c\
 src/channel/Factories.c\
 src/channel/List.c\
 src/channel/magic.c\
 src/channel/UDPChannel.c

EXE_SRCS =\
 test/c_publisher_java_subscriber_Person.c\
 test/c_publisher_java_subscriber_Shapes.c\
 test/c_subscriber_java_publisher_Person.c\
 test/coll.c\
 test/dcrudArgumentsTest.c\
 test/ioByteBufferTest.c\
 test/main.c\
 test/Person.c\
 test/poolTest.c\
 test/StaticRegistry.c\
 test-channel/channelTest.c\
 test-channel/Family.c\
 test-channel/ISO8601Date.c\
 test-channel/Person.c\
 test-channel/Types.c

ifeq ($(UNAME_S),Linux)
   LINK_FLAGS_DEBUG   = -pthread -rdynamic
   LINK_FLAGS_RELEASE = -pthread
else
   LINK_FLAGS_DEBUG   = -lws2_32
   LINK_FLAGS_RELEASE = -lws2_32
	LIB_SRCS += src/util/Sockets.c
endif

LIB_DEPS_DEBUG   = $(patsubst %.c,Debug/%.d,$(LIB_SRCS))
LIB_OBJS_DEBUG   = $(patsubst %.c,Debug/%.o,$(LIB_SRCS))
EXE_DEPS_DEBUG   = $(patsubst %.c,Debug/%.d,$(EXE_SRCS))
EXE_OBJS_DEBUG   = $(patsubst %.c,Debug/%.o,$(EXE_SRCS))

LIB_DEPS_RELEASE = $(patsubst %.c,Release/%.d,$(LIB_SRCS))
LIB_OBJS_RELEASE = $(patsubst %.c,Release/%.o,$(LIB_SRCS))
EXE_DEPS_RELEASE = $(patsubst %.c,Release/%.d,$(EXE_SRCS))
EXE_OBJS_RELEASE = $(patsubst %.c,Release/%.o,$(EXE_SRCS))

LIB_DEPS_STATIC_DEBUG   = $(patsubst %.c,Static-Debug/%.d,$(LIB_SRCS))
LIB_OBJS_STATIC_DEBUG   = $(patsubst %.c,Static-Debug/%.o,$(LIB_SRCS))
EXE_DEPS_STATIC_DEBUG   = $(patsubst %.c,Static-Debug/%.d,$(EXE_SRCS))
EXE_OBJS_STATIC_DEBUG   = $(patsubst %.c,Static-Debug/%.o,$(EXE_SRCS))

LIB_DEPS_STATIC_RELEASE = $(patsubst %.c,Static-Release/%.d,$(LIB_SRCS))
LIB_OBJS_STATIC_RELEASE = $(patsubst %.c,Static-Release/%.o,$(LIB_SRCS))
EXE_DEPS_STATIC_RELEASE = $(patsubst %.c,Static-Release/%.d,$(EXE_SRCS))
EXE_OBJS_STATIC_RELEASE = $(patsubst %.c,Static-Release/%.o,$(EXE_SRCS))

.PRECIOUS:\
 $(LIB_DEPS_STATIC_DEBUG)\
 $(EXE_DEPS_STATIC_DEBUG)\
 $(LIB_DEPS_STATIC_RELEASE)\
 $(EXE_DEPS_STATIC_RELEASE)\
 $(LIB_DEPS_DEBUG)\
 $(EXE_DEPS_DEBUG)\
 $(LIB_DEPS_RELEASE)\
 $(EXE_DEPS_RELEASE)

-include $(LIB_DEPS_STATIC_DEBUG)
-include $(EXE_DEPS_STATIC_DEBUG)
-include $(LIB_DEPS_STATIC_RELEASE)
-include $(EXE_DEPS_STATIC_RELEASE)
-include $(LIB_DEPS_DEBUG)
-include $(EXE_DEPS_DEBUG)
-include $(LIB_DEPS_RELEASE)
-include $(EXE_DEPS_RELEASE)

.PHONY: static-debug static-release debug release all clean tests coll io shapes

all: static-debug static-release debug release

static-debug: Static-Debug/dcrud-c-test
static-release: Static-Release/dcrud-c-test
debug: Debug/dcrud-c-test
release: Release/dcrud-c-test

tests: coll io shapes

clean:
	rm -fr Static-Debug
	rm -fr Static-Release
	rm -fr Debug
	rm -fr Release

#------------------------------- TESTS -------------------------------
#-- Static

ts2: Static-Debug/dcrud-c-test
	LD_LIBRARY_PATH=Static-Debug Static-Debug/dcrud-c-test --test=2

ms2: Static-Debug/dcrud-c-test
	LD_LIBRARY_PATH=Static-Debug $(VALGRIND) Static-Debug/dcrud-c-test --test=2

ts3: Static-Debug/dcrud-c-test
	LD_LIBRARY_PATH=Static-Debug Static-Debug/dcrud-c-test --test=3

ms3: Static-Debug/dcrud-c-test
	LD_LIBRARY_PATH=Static-Debug $(VALGRIND) Static-Debug/dcrud-c-test --test=3

ts4: Static-Debug/dcrud-c-test
	LD_LIBRARY_PATH=Static-Debug Static-Debug/dcrud-c-test --test=4

ms4: Static-Debug/dcrud-c-test
	LD_LIBRARY_PATH=Static-Debug $(VALGRIND) Static-Debug/dcrud-c-test --test=4

ts5: Static-Debug/dcrud-c-test
	gnome-terminal --geometry=80x60-0+0 -x java -cp ../Java/bin tests.person.Test_005 &
	LD_LIBRARY_PATH=Static-Debug Static-Debug/dcrud-c-test --test=5

ms5: Debug/dcrud-c-test
	gnome-terminal --geometry=80x60-0+0 -x java -cp ../Java/bin tests.person.Test_005 &
	LD_LIBRARY_PATH=Static-Debug $(VALGRIND) Static-Debug/dcrud-c-test --test=5

dtj6: Static-Debug/dcrud-c-test
	gnome-terminal --geometry=80x60-0+0 -x java -cp ../Java/bin tests.person.Test_006 --dump-received-buffer=true &

dts6: Static-Debug/dcrud-c-test
	LD_LIBRARY_PATH=Static-Debug Static-Debug/dcrud-c-test --test=6 --dump-received-buffer=true

ts6: Static-Debug/dcrud-c-test
	gnome-terminal --geometry=80x60-0+0 -x java -cp ../Java/bin tests.person.Test_006 &
	LD_LIBRARY_PATH=Static-Debug Static-Debug/dcrud-c-test --test=6

ms6: Debug/dcrud-c-test
	gnome-terminal --geometry=80x60-0+0 -x java -cp ../Java/bin tests.person.Test_006 &
	LD_LIBRARY_PATH=Static-Debug $(VALGRIND) Static-Debug/dcrud-c-test --test=6

#-- Dynamic

t2: Debug/dcrud-c-test
	LD_LIBRARY_PATH=Debug Debug/dcrud-c-test --test=2

m2: Debug/dcrud-c-test
	LD_LIBRARY_PATH=Debug $(VALGRIND) Debug/dcrud-c-test --test=2

t3: Debug/dcrud-c-test
	LD_LIBRARY_PATH=Debug Debug/dcrud-c-test --test=3

m3: Debug/dcrud-c-test
	LD_LIBRARY_PATH=Debug $(VALGRIND) Debug/dcrud-c-test --test=3

t4: Debug/dcrud-c-test
	LD_LIBRARY_PATH=Debug Debug/dcrud-c-test --test=4

m4: Debug/dcrud-c-test
	LD_LIBRARY_PATH=Debug $(VALGRIND) Debug/dcrud-c-test --test=4

t5: Debug/dcrud-c-test
	LD_LIBRARY_PATH=Debug Debug/dcrud-c-test --test=5

m5: Debug/dcrud-c-test
	gnome-terminal -x java -cp ../Java/bin tests.person.Test_005&
	LD_LIBRARY_PATH=Debug $(VALGRIND) Debug/dcrud-c-test --test=5

m6: Debug/dcrud-c-test
	gnome-terminal -x java -cp ../Java/bin tests.person.Test_006&
	LD_LIBRARY_PATH=Debug $(VALGRIND) Debug/dcrud-c-test --test=6

#------------------------------- BUILD -------------------------------

Static-Debug/dcrud-c-test: Static-Debug/libdcrud-c.a $(EXE_OBJS_STATIC_DEBUG)
	$(CC) -o $@ $(EXE_OBJS_STATIC_DEBUG)   -LStatic-Debug   -ldcrud-c $(LINK_FLAGS_DEBUG)

Static-Release/dcrud-c-test: Static-Release/libdcrud-c.a $(EXE_OBJS_STATIC_RELEASE)
	$(CC) -o $@ $(EXE_OBJS_STATIC_RELEASE) -LStatic-Release -ldcrud-c $(LINK_FLAGS_RELEASE)

Static-Debug/libdcrud-c.a: $(LIB_OBJS_STATIC_DEBUG)
	$(AR) rc $@ $(LIB_OBJS_STATIC_DEBUG)

Static-Release/libdcrud-c.a: $(LIB_OBJS_STATIC_RELEASE)
	$(AR) rc $@ $(LIB_OBJS_STATIC_RELEASE)

Debug/dcrud-c-test: Debug/libdcrud-c.a $(EXE_OBJS_DEBUG)
	$(CC) -o $@ $(EXE_OBJS_DEBUG)   -LDebug   -ldcrud-c $(LINK_FLAGS_DEBUG)

Release/dcrud-c-test: Release/libdcrud-c.a $(EXE_OBJS_RELEASE)
	$(CC) -o $@ $(EXE_OBJS_RELEASE) -LRelease -ldcrud-c $(LINK_FLAGS_RELEASE)

Debug/libdcrud-c.a: $(LIB_OBJS_DEBUG)
	$(AR) rc $@ $(LIB_OBJS_DEBUG)

Release/libdcrud-c.a: $(LIB_OBJS_RELEASE)
	$(AR) rc $@ $(LIB_OBJS_RELEASE)

Static-Debug/%.d: %.c
	@mkdir -p $$(dirname $@)
	@$(CC) $(CFLAGS_DEBUG)   -DSTATIC_ALLOCATION -MM -MP -MT $(subst .d,.o,$@) $< >$@

Static-Debug/%.o: %.c
	@mkdir -p $$(dirname $@)
	$(CC) $(CFLAGS_DEBUG) -c -DSTATIC_ALLOCATION $< -o $@

Static-Release/%.d: %.c
	@mkdir -p $$(dirname $@)
	@$(CC) $(CFLAGS_RELEASE) -DSTATIC_ALLOCATION -MM -MP -MT $(subst .d,.o,$@) $< >$@

Static-Release/%.o: %.c
	@mkdir -p $$(dirname $@)
	$(CC) $(CFLAGS_RELEASE) -c -DSTATIC_ALLOCATION $< -o $@

Debug/%.d: %.c
	@mkdir -p $$(dirname $@)
	@$(CC) $(CFLAGS_DEBUG)                       -MM -MP -MT $(subst .d,.o,$@) $< >$@

Debug/%.o: %.c
	@mkdir -p $$(dirname $@)
	$(CC) -c $(CFLAGS_DEBUG) $< -o $@

Release/%.d: %.c
	@mkdir -p $$(dirname $@)
	@$(CC) $(CFLAGS_RELEASE)                     -MM -MP -MT $(subst .d,.o,$@) $< >$@

Release/%.o: %.c
	@mkdir -p $$(dirname $@)
	$(CC) -c $(CFLAGS_RELEASE) $< -o $@
