#!/bin/sh
#---------------------------------------------------------------------------------------
#---------------------------------"$1"-------"$2" -------------"$3" --------------------
#                                  ^           ^                 ^
#                                  |           |                 |
# ./strip_alg_sbatch.sh  num_of_ nodes     thrds_per_rk      minutes
#---------------------------------------------------------------------------------------
if [ "$1" -lt 64 ]; then
    sbatch --partition debug --nodes "$1" --time "$3" ./strip_alg_srun.sh "$1" "$2"


elif [ "$1" -eq 64 ]; then
    sbatch --partition small --nodes "$1" --time "$3" ./strip_alg_srun.sh "$1" "$2"


else
    sbatch --partition medium --nodes "$1" --time "$3" ./strip_alg_srun.sh "$1" "$2"

fi
