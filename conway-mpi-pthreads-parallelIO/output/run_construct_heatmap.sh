#!/bin/sh
rank=8
mpicc -g -Wall -o construct_heatmap construct_heatmap.c
mpirun -np ${rank} ./construct_heatmap
