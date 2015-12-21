include ../flags.mak

LIB_SRCS =\
 src/dcrud/Arguments.cpp\
 src/dcrud/Cache.cpp\
 src/dcrud/ClassID.cpp\
 src/dcrud/Dispatcher.cpp\
 src/dcrud/GUID.cpp\
 src/dcrud/Network.cpp\
 src/dcrud/NetworkReceiver.cpp\
 src/dcrud/ParticipantImpl.cpp\
 src/dcrud/RequiredImpl.cpp

EXE_SRCS =\
 test/ShareableShape.cpp\
 test/shapes_publisher.cpp

LIB_OBJS_DEBUG   = $(patsubst %.cpp,Debug/%.o,$(LIB_SRCS))
LIB_OBJS_RELEASE = $(patsubst %.cpp,Release/%.o,$(LIB_SRCS))

EXE_OBJS_DEBUG   = $(patsubst %.cpp,Debug/%.o,$(EXE_SRCS))
EXE_OBJS_RELEASE = $(patsubst %.cpp,Release/%.o,$(EXE_SRCS))

.PHONY: all clean tests shapes

all:\
 Debug/dcrud-cpp-test\
 Release/dcrud-cpp-test

tests: shapes

clean:
	rm -fr Debug
	rm -fr Release
	rm -fr test/Debug
	rm -fr test/Release

shapes: Debug/dcrud-cpp-test
	LD_LIBRARY_PATH=Debug Debug/dcrud-cpp-test shapes --pub-id 1 --interface 192.168.1.7

../C/Debug/libdcrud-c-lib.so:
	cd ../C ; make Debug/libdcrud-c-lib.so

../C/Release/libdcrud-c-lib.so:
	cd ../C ; make Release/libdcrud-c-lib.so

Debug/dcrud-cpp-test: ../C/Debug/libdcrud-c-lib.so Debug/libdcrud-cpp-lib.so $(EXE_OBJS_DEBUG)
	$(CCC) -o $@ $(EXE_OBJS_DEBUG) -LDebug -ldcrud-cpp-lib -L../C/Debug -ldcrud-c-lib -pthread

Release/dcrud-cpp-test: ../C/Release/libdcrud-c-lib.so Release/libdcrud-cpp-lib.so $(EXE_OBJS_RELEASE)
	$(CCC) -o $@ $(EXE_OBJS_RELEASE) -LRelease -ldcrud-cpp-lib -L../C/Release -ldcrud-c-lib -pthread

Debug/libdcrud-cpp-lib.so: $(LIB_OBJS_DEBUG)
	$(CCC) -pthread -shared -o $@ $(LIB_OBJS_DEBUG)

Release/libdcrud-cpp-lib.so: $(LIB_OBJS_RELEASE)
	$(CCC) -pthread -shared -o $@ $(LIB_OBJS_RELEASE)

Debug/%.o: %.cpp
	@mkdir -p $$(dirname $@)
	$(CCC) $(CFLAGS_DEBUG) -c $< -o $@

Release/%.o: %.cpp
	@mkdir -p $$(dirname $@)
	$(CCC) $(CFLAGS_RELEASE) -c $< -o $@

