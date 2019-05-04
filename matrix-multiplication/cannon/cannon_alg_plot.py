#!/usr/bin/env python3
import numpy as np
import set_mpl as sm 
plt = sm.plt

# ==============================================================================
# Define functions 
# ==============================================================================
# ----------- Plot lines ----------------
def PlotFunt( x , y , y_lbl , scale , lbl , my_mk ,  fname ) :
    #plt.plot( x , y ,  '--b' , label = lbl , marker = 'o', ms = 8 , mfc = 'none' , mec = 'r' , mew = 2 )
    plt.plot( x , y ,  label = lbl , marker = my_mk ,  ms = 8 , mfc = 'none' , mew = 2 )

    plt.xlabel( 'MPI ranks' )
    plt.ylabel( y_lbl )
    plt.yscale( scale )
 
    # For plotting total execution, speedup, efficiency, and I/O 
    # plt.legend( loc = 'lower center' , bbox_to_anchor = ( 0.5 , 1.0 ) , ncol = 4 )

    # For plotting time distribution 
    plt.legend( loc = 'lower center' , bbox_to_anchor = ( 0.5 , 0.95 ) , ncol = 3 )

    plt.savefig( fname , format = 'png' )

# ------- Print matrix output for making latex table ---------
def PrintMatForLatex( mat_sizes , my_2d_array ) :
    n , m = my_2d_array.shape
    for i in range( n ) :
        print( '{0} & '.format( mat_sizes[ i ] ) , end="" )
        for j in range( m ) :
            if j == m - 1 :
                print( '%.3f' % my_2d_array[ i , j ] , end="" )
            else:
                print( '%.3f & ' % my_2d_array[ i , j ] , end="")
        print(' \\\\\n')


# ==============================================================================
# Initialization 
# ==============================================================================
nodes = [ 1 , 4 , 16 , 64 ]
ranks = [ i * 64 for i in nodes ] 
mat_sizes = [ 1024 , 2048 , 4096 , 8192 ]
num_nds = len( nodes )
num_mat = len( mat_sizes )

# ==============================================================================
# Read data to arrays from files
# ==============================================================================
fpath = "output/data/"
set_cart_t , init_align_t , real_t = [] , [] , []
snd_rev_t , tot_cannon_t , io_t =  [] , [] , []
for m_size in mat_sizes :
    for node in nodes :
        rks = node * 64
        fname = ''.join( ( fpath , "nds_" , str( node ) , "_rks_" , str( rks ) , \
                           "_rows_" , str( m_size ) , ".txt" ) ) 
        i = 0
        with open( fname ) as f:
            for line in f :
                lns = line.strip()
                if not lns.startswith( "#" ) and len( lns ) != 0 :
                    #print( line.rstrip() )
                    i += 1
                    if i == 1 : 
                        set_cart_t.append( float( line.rstrip() ) )
                    elif i == 2 : 
                        init_align_t.append( float( line.rstrip() ) )
                    elif i == 3 : 
                        real_t.append( float( line.rstrip() ) )
                    elif i == 4 : 
                        snd_rev_t.append( float( line.rstrip() ) )
                    elif i == 5 : 
                        tot_cannon_t.append( float( line.rstrip() ) )
                    elif i == 6 : 
                        io_t.append( float( line.rstrip() ) )
            f.close()

# ==============================================================================
# Convert lists to 2D arrays, compute speedup and efficiency 
# ==============================================================================
set_cart_t = np.reshape( set_cart_t , ( num_mat , num_nds ) )
init_align_t = np.reshape( init_align_t , ( num_mat , num_nds ) )
real_t = np.reshape( real_t , ( num_mat , num_nds ) )
snd_rev_t = np.reshape( snd_rev_t , ( num_mat , num_nds ) )
tot_cannon_t = np.reshape( tot_cannon_t , ( num_mat , num_nds ) )
io_t = np.reshape( io_t , ( num_mat , num_nds ) )
speedup = np.max( tot_cannon_t ) / tot_cannon_t
efficiency = speedup / ranks 


# ==============================================================================
# Plot data 
# ==============================================================================

my_marker = [ 'o' ,'s' , '^' , '<' ]

# --------------- Plot total cannon time vs. ranks ------------------
fname = 'execution.png'
scale = 'log'
plt.figure()
for i in range( len( mat_sizes ) ) :
    PlotFunt( ranks , tot_cannon_t[ i , : ] , 'Execution/s', scale ,\
              mat_sizes[ i ], my_marker[ i ] , fname ) 

# --------------- Plot speedup vs. ranks ------------------
fname = 'speedup.png'
#scale = 'linear'
scale = 'log'
plt.figure()
for i in range( len( mat_sizes ) ) :
    PlotFunt( ranks , np.max( tot_cannon_t ) / tot_cannon_t[ i , : ] , 'Speedup', \
              scale , mat_sizes[ i ], my_marker[ i ] , fname ) 

# --------------- Plot efficiency vs. ranks ------------------
fname = 'efficiency.png'
scale = 'log'
plt.figure()
for i in range( len( mat_sizes ) ) :
    PlotFunt( ranks , np.max( tot_cannon_t ) / tot_cannon_t[ i , : ] / ranks , \
              'Parallel Efficiency', scale , mat_sizes[ i ], my_marker[ i ] , fname ) 

# --------------- Plot I/O vs. ranks ------------------
fname = 'io.png'
scale = 'log'
plt.figure()
for i in range( len( mat_sizes ) ) :
    PlotFunt( ranks , io_t[ i , : ] , 'Parallel I/O', scale , \
              mat_sizes[ i ], my_marker[ i ] , fname ) 

# --------------- Plot overhead and real time vs. ranks ------------------
my_marker = [ 'o' ,'^' , 's' , '<' , 'x' ]
scale = 'log'
label = [ 'Total' , 'Real' , 'Comm', 'Align' , 'Topology']
for i in range( len( mat_sizes ) ) :
    fname = ''.join( ( 'time_distribute_' , str( mat_sizes[ i ] ) , '.png' ) )
    plt.figure()

    j = 0
    PlotFunt( ranks , tot_cannon_t[ i , : ], 'Time/s', scale , \
              label[ j ], my_marker[ j ] , fname ) 
    j += 1
    PlotFunt( ranks , real_t[ i , : ], 'Time/s', scale , \
              label[ j ], my_marker[ j ] , fname ) 

    j += 1
    PlotFunt( ranks , snd_rev_t[ i , : ], 'Time/s', scale , \
              label[ j ], my_marker[ j ] , fname ) 

    j += 1
    PlotFunt( ranks , init_align_t[ i , : ], 'Time/s', scale , \
              label[ j ], my_marker[ j ] , fname ) 
    j += 1
    PlotFunt( ranks , set_cart_t[ i , : ], 'Time/s', scale , \
              label[ j ], my_marker[ j ] , fname ) 
plt.show()

# ==============================================================================
# Print data for latex 
# ==============================================================================
"""
print( "tot_cannon_t: " )
PrintMatForLatex( mat_sizes, tot_cannon_t ) 
print( "\n\nspeedup: " )
PrintMatForLatex( mat_sizes, speedup ) 
print( "\n\neffiency: " )
PrintMatForLatex( mat_sizes, efficiency ) 
print( "\n\nI/O: " )
PrintMatForLatex( mat_sizes, io_t ) 
print( "\n\nCommunication: " )
PrintMatForLatex( mat_sizes, snd_rev_t ) 
print( "\n\nSetup cart: " )
PrintMatForLatex( mat_sizes, set_cart_t ) 
print( "\n\init_align_t: " )
PrintMatForLatex( mat_sizes, init_align_t ) 
print( "\n\real_t: " )
PrintMatForLatex( mat_sizes, real_t ) 
#np.set_printoptions( precision = 3 )
#np.set_printoptions( suppress = True )
"""
