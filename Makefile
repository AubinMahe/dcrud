all:\
 C/Debug/dcrud-c-test\
 C/Release/dcrud-c-test\
 cpp/Debug/dcrud-cpp-test\
 cpp/Release/dcrud-cpp-test\
 Java/dcrud.jar\
 Java/dcrud-ui.jar

clean:
	rm -fr C/Debug
	rm -fr C/Release
	rm -fr cpp/Debug
	rm -fr cpp/Release
	rm -fr Java/bin
	cd Java ; ant clean
	rm -fr VisualStudioProject/x64
	rm -fr VisualStudioProject/dcrud/x64
	rm -fr VisualStudioProject/dcrud-cpp/x64
	rm -fr VisualStudioProject/test/x64
	rm -fr VisualStudioProject/test-cpp/x64
	rm -f  VisualStudioProject/dcrud.sdf

tests: Test_001 Test_002 Test_003 Test_004 Test_005 Test_006 Test_007 Test_008

Test_%:
	cd C ; make
	cd cpp ; make
	cd Java ; ant $@ 

C/Debug/dcrud-c-test:
	cd C ; make Debug/dcrud-c-test

C/Release/dcrud-c-test:
	cd C ; make Release/dcrud-c-test

cpp/Debug/dcrud-cpp-test:
	cd cpp ; make Debug/dcrud-cpp-test

cpp/Release/dcrud-cpp-test:
	cd cpp ; make Release/dcrud-cpp-test

Java/dcrud.jar:
	cd Java ; ant jar-lib

Java/dcrud-ui.jar:
	cd Java ; ant jar-test
