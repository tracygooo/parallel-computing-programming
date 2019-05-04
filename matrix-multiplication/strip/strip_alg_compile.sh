#!/bin/sh
#-------------------------------------------------------------
# ./strip_alg_compile.sh
#-------------------------------------------------------------
module load xl
mpicc -O3 -o strip_alg strip_alg.c -lpthread 
