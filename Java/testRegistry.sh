#!/bin/bash

for(( i=1; i<=$1; i++ ))
do
   ./launchOneRegistry.sh $i &
done
