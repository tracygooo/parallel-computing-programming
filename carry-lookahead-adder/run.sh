#!/bin/bash

#echo on
set -x 

for i in {0..7}
do 
    #./cla < input/t"$i".txt > output/cla/c"$i".txt 
    diff output/cla/c"$i".txt output/answer/A"$i".txt 
done
