#!/bin/bash

#===============================================================
# ./run_diff_rank.sh $1 $2 
#    $1 ( whether adding barrier or not, depending on mpi.c ) 
#    $2 ( which test case: 1-7 )
#===============================================================

# barrier = 0 in code file mpi.c
if [ "$1" = 0 ] ; then 
    echo "With MPI_Barrier()"

# barrier != 0 in code file mpi.c
else
    echo "Without MPI_Barrier()"
fi

#------------------- Compile ---------------------------
mpicc -Wall -g -o ./code/mpi ./code/mpi.c 

#------------n: number of processors--------------------
for n in {1,2,4,8,16,32}
do 
    #------------------- Execute and output results ---------------------------
    mpirun -np "$n" code/mpi input/test_input_"$2".txt output/mpi/my_output_1.txt
done
