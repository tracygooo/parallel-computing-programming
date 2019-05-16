#!/bin/sh
fname="convert.txt"
thrh=0.5 # threshold
srun --ntasks-per-node=64 --overcommit -o "$fname" ./convert_bin_to_txt 128 4 ${thrh} 128 
