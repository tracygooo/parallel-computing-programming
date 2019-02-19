#!/bin/bash

#------echo on-------
#set -x 

#------------------- Compile ---------------------------
mpicc -Wall -g -o ./code/mpi ./code/mpi.c 

for n in {1,2,4,8,16,32}
do 
    #------------------- Execute and output results ---------------------------
    mpirun -np "$n" code/mpi input/test_input_1.txt output/mpi/my_output_1.txt
done
