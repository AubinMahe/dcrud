#!/bin/bash

gnuplot -e "activity='publish'" histo.gnu
gnuplot -e "activity='refresh'" histo.gnu
gnuplot -e "activity='network'" histo.gnu

