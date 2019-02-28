#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Plot execution time of 64, 128, 256, ... , 8192 ranks in Blue Gene/Q 
Compare execution times of point2point and collective methods
"""

#import sys
import numpy as np
import set_mpl as sm
plt = sm.plt

__author__ = "Jinghua Feng"
__copyright__ = "Copyright 2019, Plots of P2P"
__credits__ = ["Jinghua Feng"]
__license__ = "GPL"
__version__ = "0.1.0"
__maintainer__ = "Jinghua Feng"
__email__ = "tracygooo@gmail.com"
__status__ = "Dev"

def PlotFunt( x , y , method , ylb ) :
    if method is 0 :
        plt.plot( x , y ,  '--r' , label = r'with MPI\_reduce()', marker = 'o', \
                  ms = 8 , mfc = 'none' , mec = 'r' , mew = 2 )
    else :
        plt.plot( x , y ,  '-b' , label = r'without MPI\_P2P\_reduce()', marker = '^', \
                  ms = 8 , mfc = 'none' , mec = 'b' , mew = 2 )
    plt.xlabel( 'Rank' )
    plt.ylabel( ylb )
    plt.legend()

data = np.loadtxt( "./output.txt" )

#----------------------Execution vs rank------------------------
plt.figure( 0 )

# MPI_reduce
PlotFunt( data[ 0:, 0 ] , data[ 0:, 1 ] , 0 ,  "Execution/s" )

# MPI_P2P_reduce
PlotFunt( data[ 0:, 0 ] , data[ 0:, 2 ] , 1 ,  "Execution/s" )

#plt.title( "Execution time at different ranks" )
plt.tight_layout()
plt.savefig( "exe_vs_rank.png" , bbox_inches = "tight" , format = 'png' )

plt.show()
