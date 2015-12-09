#!/bin/bash

REPORT='vg-shapes-report.txt'
VG_OPTS='--leak-check=full --show-leak-kinds=all'
ARGS='--pub-name Shapes-1 --interface 192.168.1.5'
valgrind $VG_OPTS Debug/dcrud shapes $ARGS 2> $REPORT
echo Results in $REPORT
