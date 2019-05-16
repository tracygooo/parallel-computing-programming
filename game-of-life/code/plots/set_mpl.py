import matplotlib as mpl
font = { 'size' : 16 }
mpl.rc( 'xtick' , labelsize = 16 )
mpl.rc( 'ytick' , labelsize = 16 )
mpl.rc( 'font' , **font )
import matplotlib.pyplot as plt
plt.rc('text' , usetex = True )
