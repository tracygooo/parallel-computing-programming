#!/bin/bash

#===============================================================
# ./run_diff_rank.sh 
# run p2p.o with 64, 128, 256, ... , 8192 MPI ranks
#===============================================================

echo -e "collective \t Point2Point"
#------------------- Compile ---------------------------
#module load xl
# mpicc -03 p2p.c -lm -o p2p
#mpicc -g p2p.c -lm -o p2p
#mpicxx -03 p2p.c -lm -o p2p.o

#----n: number of processors; t: time limit--------------
n=64
t=1

#while [ "$n" -le 8192 ]
while [ $n -le 64 ]
do  
    #------------------- Execute and output results ---------------------------
    echo -e "$n MPI ranks:"
    srun -n "$n" --time "$t" ./p2p
    echo -e "\n\n"
    n=$(( n*2 ))
done

# printf "$n\t"
# srun -n "$n" --time "$t" -o none ./p2p
# t=$( echo "$t/2" | bc -l )
# echo "t is $t"
