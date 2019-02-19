#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Plot execution time of 2, 4, 8, 16 and 32 rank
Plot speedup relative to serial MPI CLA adder 
Plot speedup relative to serial Ripple carry
"""

#import sys
import numpy as np
import set_mpl as sm
plt = sm.plt

__author__ = "Jinghua Feng"
__copyright__ = "Copyright 2019, Plots of MPI CLA"
__credits__ = ["Jinghua Feng"]
__license__ = "GPL"
__version__ = "0.1.0"
__maintainer__ = "Jinghua Feng"
__email__ = "tracygooo@gmail.com"
__status__ = "Dev"

def PlotFunt( x , y , barrier , ylb ) :
    if barrier is 0 :
        plt.plot( x , y ,  '--r' , label = r'with MPI\_Barrier()', marker = 'o', \
                  ms = 8 , mfc = 'none' , mec = 'r' , mew = 2 )
    else :
        plt.plot( x , y ,  '-b' , label = r'without MPI\_Barrier()', marker = '^', \
                  ms = 8 , mfc = 'none' , mec = 'b' , mew = 2 )
    plt.xlabel( 'Rank' )
    plt.ylabel( ylb )
    plt.legend()

data_barr = np.loadtxt( "../output/execution/t1_barrier.txt" )
data_no_barr = np.loadtxt( "../output/execution/t1_no_barrier.txt" )
serial_ripple = np.loadtxt( "../output/execution/ripple.txt" )

serial_ripple = serial_ripple.reshape( ( 1 ) )
serial_CLA_barr = np.array( [ data_barr[ 0 , 1 ] ] )
serial_CLA_no_barr = np.array( [ data_no_barr[ 0 , 1 ] ] )

#----------------------Execution vs rank------------------------
plt.figure( 0 )
PlotFunt( data_barr[ 1:, 0 ] , data_barr[ 1:, 1 ] , 0 ,  "Execution/s" )
PlotFunt( data_no_barr[ 1:, 0 ] , data_no_barr[ 1:, 1 ] , -1 , "Execution/s" )
#plt.title( "Execution time at different ranks" )
plt.tight_layout()
plt.savefig( "exe_vs_rank.png" , bbox_inches = "tight" , format = 'png' )

#---------------Speedup relative to serial CLA-----------------
plt.figure( 1 )
PlotFunt( data_barr[ 1: , 0 ] , np.divide( serial_CLA_barr , data_barr[ 1: , 1 ] ) , 0 , "Speedup"  )
PlotFunt( data_no_barr[ 1: , 0 ] , np.divide( serial_CLA_no_barr , data_no_barr[ 1: , 1 ] ) , -1 , "Speedup" )
#plt.title( "Speedup relative to serial MPI CLA" )
plt.tight_layout()
plt.savefig( "speedup_rel_MPI_CLA.png" , format = 'png' )

#---------------Speedup relative to serial Ripple-----------------
plt.figure( 2 )
PlotFunt( data_barr[ 1: , 0 ] , np.divide( serial_ripple , data_barr[ 1: , 1 ] ) , 0 , "Speedup" )
PlotFunt( data_no_barr[ 1: , 0 ] , np.divide( serial_ripple , data_no_barr[ 1: , 1 ] ) , -1 , "Speedup" )
#plt.title( "Speedup relative to serial ripple" )
plt.tight_layout()
plt.savefig( "speedup_rel_serial_ripple.png" , format = 'png' )

plt.show()
