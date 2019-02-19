import matplotlib as mpl
font = { 'size' : 20 }
mpl.rc( 'xtick' , labelsize = 20 )
mpl.rc( 'ytick' , labelsize = 20 )
mpl.rc( 'font' , **font )
import matplotlib.pyplot as plt
plt.rc('text' , usetex = True )
