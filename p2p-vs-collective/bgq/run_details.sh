#!/bin/bash

#===============================================================
# ./run_diff_rank.sh 
# run p2p.o with 64, 128, 256, ... , 8192 MPI ranks
#===============================================================

#------------------- Compile ---------------------------
module load xl
mpicc -g p2p.c -lm -o p2p_details
#mpicxx -03 p2p.c -lm -o p2p.o

#------------n: number of processors--------------------
n=64
t=1
while [ "$n" -le 8192 ]
#while [ $n -le 128 ]
do  
    #------------------- Execute and output results ---------------------------
    echo -e "$n MPI ranks:"
    srun -n "$n" --time "$t" ./p2p_details
    echo -e "\n\n"
    n=$(( n*2 ))
    # t=$( echo "$t/2" | bc -l )
    #echo "t is $t"
done
