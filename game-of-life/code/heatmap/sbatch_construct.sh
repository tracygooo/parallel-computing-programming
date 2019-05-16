#!/bin/sh
#-----------------------------------------------------
# ./sbatch_construct_heatmap time_limit
#-----------------------------------------------------
sbatch --partition debug --nodes 16 --time "$1" ./srun_construct_heatmap.sh  
