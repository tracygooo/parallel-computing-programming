#!/bin/sh

#===============================================================
# Run "./sbatch_var.sh node_num iter_num  
#===============================================================


# ------------ Compile -----------------
# module load xl
# mpicc -g p2p.c -lm -o p2p.xl

let "rank = 64 * $1"
for ((i = 1; i <= $2; i++ ))
do
    if [ "$1" -lt 64 ]; then
        sbatch --partition debug --nodes "$1" --time 1 ./run_var.sh "$rank" "$i"

    elif [ "$1" -eq 64 ]; then
        sbatch --partition small --nodes 64 --time 1 ./run_var.sh "$rank" "$i"

    else
        sbatch --partition medium --nodes 128 --time 1 ./run_var.sh "$rank" "$i"
    fi
done
