#!/bin/bash

java -cp bin -agentlib:jdwp=transport=dt_socket,server=y,suspend=n,address=$((8000+$1))\
 tests.registry.GUI\
 --interface=eth0\
 --registry-host=192.168.1.7\
 --registry-port=2416\
 --boot-port=2416\
 --rank=$1
