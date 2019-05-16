#!/bin/sh
fname="construct_heatmap.txt"
thrh=0.5 # threshold
srun --ntasks-per-node=64 --overcommit -o "$fname" /gpfs/u/home/PCP8/PCP8fngj/scratch/game-of-life/output/construct_heatmap 128 4 ${thrh} 128 
