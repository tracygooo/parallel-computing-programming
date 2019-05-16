#!/bin/sh
#-------------------------------------------------------------
# ./my_compile.sh
#-------------------------------------------------------------
module load xl
mpicc -O3 -o convert_bin_to_txt convert_bin_to_txt.c 
