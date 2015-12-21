all:\
 C/test/Debug/dcrud-c-test\
 C/test/Release/dcrud-c-test\
 cpp/test/Debug/dcrud-cpp-test\
 cpp/test/Release/dcrud-cpp-test\
 Java/dcrud.jar\
 Java/dcrud-ui.jar

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

C/test/Debug/dcrud-c-test:
	cd C ; make Debug/dcrud-c-test

C/test/Release/dcrud-c-test:
	cd C ; make Release/dcrud-c-test

cpp/test/Debug/dcrud-cpp-test:
	cd cpp ; make Debug/dcrud-cpp-test

cpp/test/Release/dcrud-cpp-test:
	cd cpp ; make Release/dcrud-cpp-test

Java/dcrud.jar:
	cd Java ; ant jar-lib

Java/dcrud-ui.jar:
	cd Java ; ant jar-test
