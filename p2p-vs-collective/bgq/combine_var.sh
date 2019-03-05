#!/bin/sh

#===============================================================
# Run "./combine_var rank_size tot_num 
#===============================================================

# Clear file "output_var.txt"
> output_var.txt

# Copy contents of files "output_$1_$i" to file "output_var.txt"
for (( i = 1 ; i <= $2; i++ ))
do
   cat "output_${1}_${i}" >> output_var.txt 
done
