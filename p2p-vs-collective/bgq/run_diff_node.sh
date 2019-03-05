#!/bin/sh

fname="output_"
let "rank = 64 * $1"
srun --ntasks-per-node=64 --overcommit -o "$fname$rank" /gpfs/u/home/PCP8/PCP8fngj/barn/p2p.xl
