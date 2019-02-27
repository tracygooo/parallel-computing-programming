#!/bin/bash

#===============================================================
# ./run_diff_rank.sh 
# run p2p.o with 64, 128, 256, ... , 8192 MPI ranks
#===============================================================

#------------------- Compile ---------------------------
mpicc -Wall -g p2p.c -lm -o p2p.o

#------------n: number of processors--------------------
n=64
#while [ "$n" -le 8192 ]
while [ $n -le 128 ]
do 
    #------------------- Execute and output results ---------------------------
    echo -e "$n MPI ranks:"
    mpirun -np "$n" ./p2p.o
    echo -e "\n\n"
    n=$(( n*2 ))
done
