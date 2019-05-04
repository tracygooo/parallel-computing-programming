#!/bin/sh
#---------------------------------------------------------
# Run by ./cannon_alg_sbatch.sh num_of_nodes matrix_rows 
#---------------------------------------------------------
let "ntasks = 64" # number of tasks per node
let "rks = ${1} * ntasks"
srun --ntasks-per-node=${ntasks} --overcommit -o "nds_${1}_rks_${rks}_rows_${2}.txt" ./cannon_alg ${2} ${ntasks}
