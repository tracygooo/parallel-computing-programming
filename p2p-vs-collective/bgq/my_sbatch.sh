#!/bin/sh

#===============================================================
# Run "./my_sbatch.sh compile" if compiling is needed  
# Execute run_p2p.sh with papameters 1,2,4,8,...,128 nodes
#===============================================================

# --- Num of nodes ---
n=1

while [ "$n" -le 128 ]
#while [ "$n" -le 1 ]
do
	# ------------ check below website for "--partition" -----------------------
	# https://secure.cci.rpi.edu/wiki/index.php/Blue_Gene/Q#Partitions
    # --------------------------------------------------------------------------
    if [ "$n" -lt 64 ]
        sbatch --partition debug --nodes "$n" --time 1 ./run_p2p.sh "$n" $1
    fi

    if [ "$n" -eq 64 ]
        sbatch --partition small --nodes 64 --time 1 ./run_p2p.sh 64
    fi

    if [ "$n" -eq 128 ]
        sbatch --partition medium --nodes 128 --time 1 ./run_p2p.sh 128
    fi

    n=$(( n*2 ))

done
