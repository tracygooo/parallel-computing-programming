#!/bin/bash

fname="p2p"
let "n=64 * $1"
#if [ "$1" -eq 1 ]; then 
if [ "$1" = "print" ]; then 
    printf "$fname$n.txt"
else
    printf "no para"
fi
