#!/bin/sh
#-------------------------------------------------------------
# ./cannon_alg_compile.sh
#-------------------------------------------------------------
module load xl
mpicc -O3 -o cannon_alg cannon_alg.c -lm
