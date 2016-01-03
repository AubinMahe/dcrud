#!/bin/bash

find Java/test -type f -name '*.java' -exec grep -l "$1" {} \;
find cpp/test  -type f -name '*.cpp'  -exec grep -l "$1" {} \;
find C/test    -type f -name '*.c'    -exec grep -l "$1" {} \;

