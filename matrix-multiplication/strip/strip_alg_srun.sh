#!/bin/sh
#-----------------------------------------------------------------------
# ./strip_alg_srun.sh num_of_nodes thrds_per_rk
# Run by ./strip_alg_sbatch.sh num_of_nodes thrds_per_rk time_limit
#-----------------------------------------------------------------------
let "tpr = ${2}" # number of threads per rank
let "ntasks = 64 / ${tpr}" # number of tasks per node
srun --ntasks-per-node=${ntasks} --overcommit -o "strip_nds_${1}_tpr_${tpr}.txt" ./strip_alg ${tpr} 
