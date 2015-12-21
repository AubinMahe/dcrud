include ../flags.mak

LIB_SRCS =\
 src/os/System.c\
 src/os/Mutex.c\
 src/dcrud/Arguments.c\
 src/dcrud/ParticipantImpl.c\
 src/dcrud/Network.c\
 src/dcrud/Dispatcher.c\
 src/dcrud/Status.c\
 src/dcrud/Shareable.c\
 src/dcrud/NetworkReceiver.c\
 src/dcrud/ClassID.c\
 src/dcrud/Callback.c\
 src/dcrud/Cache.c\
 src/dcrud/GUID.c\
 src/util/CheckSysCall.c\
 src/io/Errors.c\
 src/io/ByteBuffer.c\
 src/coll/MapVoidPtr.c\
 src/coll/Set.c\
 src/coll/Comparator.c\
 src/coll/MapFuncPtr.c\
 src/coll/List.c\
 src/dbg/Performance.c\
 src/dbg/Dump.c

EXE_SRCS =\
 test/main.c\
 test/test_002.c\
 test/test_003.c\
 test/test_006.c\
 test/test_008.c

LIB_OBJS_DEBUG   = $(patsubst %.c,Debug/%.o,$(LIB_SRCS))
LIB_OBJS_RELEASE = $(patsubst %.c,Release/%.o,$(LIB_SRCS))

EXE_OBJS_DEBUG   = $(patsubst %.c,Debug/%.o,$(EXE_SRCS))
EXE_OBJS_RELEASE = $(patsubst %.c,Release/%.o,$(EXE_SRCS))

.PHONY: all clean tests coll io shapes

all:\
 Debug/dcrud-c-test\
 Release/dcrud-c-test

tests: coll io shapes

clean:
	rm -fr Debug
	rm -fr Release
	rm -fr test/Debug
	rm -fr test/Release

coll: Debug/dcrud-c-test
	LD_LIBRARY_PATH=Debug Debug/dcrud-c-test coll

io: Debug/dcrud-c-test
	LD_LIBRARY_PATH=Debug Debug/dcrud-c-test io

shapes: Debug/dcrud-c-test
	LD_LIBRARY_PATH=Debug Debug/dcrud-c-test shapes --pub-id 1 --interface 192.168.1.7

Debug/dcrud-c-test: Debug/libdcrud-c-lib.so $(EXE_OBJS_DEBUG)
	$(CC) -o $@ $(EXE_OBJS_DEBUG) -LDebug -ldcrud-c-lib -pthread

Release/dcrud-c-test: Release/libdcrud-c-lib.so $(EXE_OBJS_RELEASE)
	$(CC) -o $@ $(EXE_OBJS_RELEASE) -LRelease -ldcrud-c-lib -pthread

Debug/libdcrud-c-lib.so: $(LIB_OBJS_DEBUG)
	$(CC) -pthread -shared -o $@ $(LIB_OBJS_DEBUG)

Release/libdcrud-c-lib.so: $(LIB_OBJS_RELEASE)
	$(CC) -pthread -shared -o $@ $(LIB_OBJS_RELEASE)

Debug/%.o: %.c
	@mkdir -p $$(dirname $@)
	$(CC) $(CFLAGS_DEBUG) -c $< -o $@

Release/%.o: %.c
	@mkdir -p $$(dirname $@)
	$(CC) $(CFLAGS_RELEASE) -c $< -o $@

