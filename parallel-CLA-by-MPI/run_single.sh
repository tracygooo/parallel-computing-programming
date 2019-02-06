#!/bin/bash

#echo on
set -x 

#------------------- Compile ---------------------------
gcc -Wall -o mpi mpi.c

#for i in {0..8}
for i in {0..0}
do 
    #------------------- Execute and output results ---------------------------
    ./mpi < t"$i".txt > m"$i".txt 
    #------------------- Compare results with answers ---------------------------
    #diff m"$i".txt A"$i".txt 
done
