#!/bin/sh
if [ "$2" == "compile" ]; then
	module load xl
	mpicc -g p2p.c -lm -o p2p.xl
fi
fname="output_"
let "rank = 64 * $1"
srun --ntasks-per-node=64 --overcommit -o "$fname$rank" /gpfs/u/home/PCP8/PCP8fngj/barn/p2p.xl
