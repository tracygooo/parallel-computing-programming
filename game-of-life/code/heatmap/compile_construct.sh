#!/bin/sh
#-------------------------------------------------------------
# ./my_compile.sh
#-------------------------------------------------------------
module load xl
mpicc -O3 -o construct_heatmap construct_heatmap.c 
