#!/bin/bash

#echo on
set -x 

#------------------- Compile ---------------------------
gcc -Wall -o mpi mpi.c

for i in {0..8}
do 
    #------------------- Execute and output results ---------------------------
    ./mpi < input/t"$i".txt > output/mpi/c"$i".txt 
    #------------------- Compare results with answers ---------------------------
    diff output/mpi/c"$i".txt output/answer/A"$i".txt 
done
