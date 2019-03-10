#!/bin/sh

#===============================================================
# Run "./run_var rank_size iter  
#===============================================================
fname="output_${1}_${2}"
srun --ntasks-per-node=64 --overcommit -o "$fname" /gpfs/u/home/PCP8/PCP8fngj/barn/p2p.xl
