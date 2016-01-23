#!/bin/bash

for(( i=1; i<=12; i++ ))
do
   java -cp bin tests.registry.GUI --host=127.0.0.1 --port=2416 --rank=$i &
done
