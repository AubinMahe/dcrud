CCC             = g++
CPPFLAGS_COMMON = -I ../C/inc -I inc
CPPFLAGS_DEBUG  = $(CPPFLAGS_COMMON)\
 -std=c++11\
 -g -Wall -Weffc++ -pedantic -pedantic-errors -Wextra\
 -Wcast-align -Wcast-qual -Wconversion\
 -O -Wdisabled-optimization\
 -Werror -Wfloat-equal\
 -Wimport -Winit-self\
 -Wno-long-long\
 -Wmissing-field-initializers\
 -Wmissing-include-dirs -Winline -Wstrict-aliasing=2\
 -Wmissing-noreturn\
 -Wpointer-arith\
 -Wredundant-decls\
 -Wstack-protector\
 -Wswitch-default -Wswitch-enum\
 -Wunreachable-code -Wunused -Wunused-parameter\
 -Wvariadic-macros\
 -Wwrite-strings

CPPFLAGS_RELEASE = $(CPPFLAGS_COMMON) -O3
VALGRIND = valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes
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
 test/shapes_publisher.cpp\
 test/StaticRegistry.cpp

LIB_DEPS_DEBUG   = $(patsubst %.cpp,Debug/%.d,$(LIB_SRCS))
LIB_OBJS_DEBUG   = $(patsubst %.cpp,Debug/%.o,$(LIB_SRCS))
EXE_DEPS_DEBUG   = $(patsubst %.cpp,Debug/%.d,$(EXE_SRCS))
EXE_OBJS_DEBUG   = $(patsubst %.cpp,Debug/%.o,$(EXE_SRCS))

LIB_DEPS_RELEASE = $(patsubst %.cpp,Release/%.d,$(LIB_SRCS))
LIB_OBJS_RELEASE = $(patsubst %.cpp,Release/%.o,$(LIB_SRCS))
EXE_DEPS_RELEASE = $(patsubst %.cpp,Release/%.d,$(EXE_SRCS))
EXE_OBJS_RELEASE = $(patsubst %.cpp,Release/%.o,$(EXE_SRCS))

.PHONY: debug release all clean tests shapes

all: debug release
debug: Debug/dcrud-cpp-test
release: Release/dcrud-cpp-test

-include $(LIB_DEPS_DEBUG)
-include $(EXE_DEPS_DEBUG)
-include $(LIB_DEPS_RELEASE)
-include $(EXE_DEPS_RELEASE)

tests: shapes

clean:
	rm -fr Debug
	rm -fr Release
	rm -fr test/Debug
	rm -fr test/Release

shapes: Debug/dcrud-cpp-test
	LD_LIBRARY_PATH=Debug Debug/dcrud-cpp-test shapes --pub-id 1 --interface 192.168.1.7

../C/Debug/libdcrud-c.a ../C/Debug/libdcrud-c.so:
	cd ../C ; make debug

../C/Release/libdcrud-c.a ../C/Release/libdcrud-c.so:
	cd ../C ; make release

Debug/dcrud-cpp-test: ../C/Debug/libdcrud-c.a Debug/libdcrud-cpp.a $(EXE_OBJS_DEBUG)
	$(CCC) -o $@ $(EXE_OBJS_DEBUG) -LDebug -ldcrud-cpp -L../C/Debug -ldcrud-c -pthread

Release/dcrud-cpp-test: ../C/Release/libdcrud-c.a Release/libdcrud-cpp.a $(EXE_OBJS_RELEASE)
	$(CCC) -o $@ $(EXE_OBJS_RELEASE) -LRelease -ldcrud-cpp -L../C/Release -ldcrud-c -pthread

Debug/libdcrud-cpp.so: $(LIB_OBJS_DEBUG)
	$(CCC) -pthread -shared -o $@ $(LIB_OBJS_DEBUG)

Debug/libdcrud-cpp.a: $(LIB_OBJS_DEBUG)
	$(AR) rc $@ $(LIB_OBJS_DEBUG)

Release/libdcrud-cpp.so: $(LIB_OBJS_RELEASE)
	$(CCC) -pthread -shared -o $@ $(LIB_OBJS_RELEASE)

Release/libdcrud-cpp.a: $(LIB_OBJS_RELEASE)
	$(AR) rc $@ $(LIB_OBJS_RELEASE)

Debug/%.d: %.cpp
	@mkdir -p $$(dirname $@)
	@$(CCC) $(CPPFLAGS_DEBUG)                       -MM -MP -MT $(subst .d,.o,$@) $< >$@

Debug/%.o: %.cpp
	@mkdir -p $$(dirname $@)
	$(CCC) $(CPPFLAGS_DEBUG) -c $< -o $@

Release/%.d: %.cpp
	@mkdir -p $$(dirname $@)
	@$(CCC) $(CPPFLAGS_RELEASE)                     -MM -MP -MT $(subst .d,.o,$@) $< >$@

Release/%.o: %.cpp
	@mkdir -p $$(dirname $@)
	$(CCC) $(CPPFLAGS_RELEASE) -c $< -o $@
