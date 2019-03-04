#!/usr/bin/env python3

import numpy as np

fname = "output_var.txt"
data = np.loadtxt( fname )

print( np.var( data , axis = 0 ) )
