all: C/test/Debug/dcrud-c-test cpp/test/Debug/dcrud-cpp-test Java/dcrud.jar Java/dcrud-ui.jar


clean:
	rm -fr C/Debug
	rm -fr C/Release
	rm -fr C/test/Debug
	rm -fr C/test/Release
	rm -fr cpp/Debug
	rm -fr cpp/Release
	rm -fr cpp/test/Debug
	rm -fr cpp/test/Release

C/Debug/libdcrud-c-lib.so:
	cd C/Debug ; make all

C/test/Debug/dcrud-c-test: C/Debug/libdcrud-c-lib.so
	cd C/test/Debug ; make all

cpp/Debug/libdcrud-cpp-lib.so:
	cd cpp/Debug ; make all

cpp/test/Debug/dcrud-cpp-test: cpp/Debug/libdcrud-cpp-lib.so
	cd cpp/test/Debug ; make all

Java/dcrud.jar:
	cd Java ; ant jar-lib

Java/dcrud-ui.jar:
	cd Java ; ant jar-test
