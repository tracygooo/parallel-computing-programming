#!/usr/bin/env python3

import numpy as np
import set_mpl as sm 

plt = sm.plt

def PlotFunt( x , y ) :
    plt.plot( x , y ,  '--b' , label = 'nodes: 128, threshold: 25\%, ticks: 256', marker = 'o', ms = 8 , mfc = 'none' , mec = 'r' , mew = 2 )
    plt.xlabel( 'MPI ranks' )
    plt.ylabel( 'I/O time' )
    plt.ylim( 0, 25 )
    plt.legend()
    fname = 'io_time.png' 
    plt.savefig( fname , format = 'png' )
    plt.show()

# Load text file
data = np.loadtxt( 'io_time.txt' )

# Convert to # of mpi ranks
x = 64 / data[:,0] * 128

# Plot io execution
PlotFunt( x , data[ : , 1 ] ) 
