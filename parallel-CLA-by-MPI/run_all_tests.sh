#!/bin/bash

#------echo on-------
set -x 

#------------------- Compile ---------------------------
mpicc -Wall -g -o ./code/mpi ./code/mpi.c 
gcc -Wall -g -o ./code/compare ./code/comp_two_files.c 

#---------- Number of processors/ranks------------------
n=2

for i in {1..7}
do 
    #------------------- Execute and output results ---------------------------
    mpirun -np "$n" code/mpi input/test_input_"$i".txt output/mpi/my_output_"$i".txt

    #------------------- Compare results with answers ---------------------------
    ./code/compare output/mpi/my_output_"$i".txt output/answer/test_output_"$i".txt 
done
