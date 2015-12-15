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
	rm -fr Java/bin
	cd Java ; ant clean
	rm -fr VisualStudioProject/x64
	rm -fr VisualStudioProject/dcrud/x64
	rm -fr VisualStudioProject/dcrud-cpp/x64
	rm -fr VisualStudioProject/test/x64
	rm -fr VisualStudioProject/test-cpp/x64
	rm -f  VisualStudioProject/dcrud.sdf

C/test/Debug/dcrud-c-test: C/Debug/libdcrud-c-lib.so
	cd C/test ; make Debug

C/Debug/libdcrud-c-lib.so:
	cd C ; make Debug

cpp/test/Debug/dcrud-cpp-test: cpp/Debug/libdcrud-cpp-lib.so
	cd cpp/test ; make Debug

cpp/Debug/libdcrud-cpp-lib.so:
	cd cpp ; make Debug

Java/dcrud.jar:
	cd Java ; ant jar-lib

Java/dcrud-ui.jar:
	cd Java ; ant jar-test

