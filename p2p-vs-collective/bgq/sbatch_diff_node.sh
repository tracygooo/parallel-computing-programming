#!/bin/sh

#===============================================================
# Run "./my_sbatch.sh compile" if compiling is needed  
# Execute run_diff_node.sh with papameters 1,2,4,8,...,128 nodes
#===============================================================

# --- Num of nodes ---
n=1

if [ "$1" == "compile" ]; then
	module load xl
	mpicc -g p2p.c -lm -o p2p.xl
fi

#while [ "$n" -le 128 ]
while [ "$n" -le 1 ]
do
	# ------------ check below website for "--partition" -----------------------
	# https://secure.cci.rpi.edu/wiki/index.php/Blue_Gene/Q#Partitions
    # --------------------------------------------------------------------------
    if [ "$n" -lt 64 ]; then
        sbatch --partition debug --nodes "$n" --time 1 ./run_diff_node.sh "$n"

    elif [ "$n" -eq 64 ]; then
        sbatch --partition small --nodes 64 --time 1 ./run_diff_node.sh 64

    else
        sbatch --partition medium --nodes 128 --time 1 ./run_diff_node.sh 128
    fi

    n=$(( n*2 ))

done
