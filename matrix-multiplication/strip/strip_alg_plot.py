import matplotlib.pyplot as plt

SMALL_SIZE = 8
MEDIUM_SIZE = 10
BIGGER_SIZE = 18

plt.rc('font', size=BIGGER_SIZE)          # controls default text sizes
plt.rc('axes', titlesize=BIGGER_SIZE)     # fontsize of the axes title
plt.rc('axes', labelsize=BIGGER_SIZE)    # fontsize of the x and y labels
plt.rc('xtick', labelsize=BIGGER_SIZE)    # fontsize of the tick labels
plt.rc('ytick', labelsize=BIGGER_SIZE)    # fontsize of the tick labels
plt.rc('legend', fontsize=MEDIUM_SIZE)    # legend fontsize
plt.rc('figure', titlesize=BIGGER_SIZE)  # fontsize of the figure title


####################################################################################################
##  Figure 1: Plot X: the total number of MPI ranks * Pthreads per rank Y: compute execution time ##
####################################################################################################
total_rank_number = [256, 512, 1024, 4096, 8192]

ratio_4 =[110.003835,65.929427,35.787762,17.894622,9.466195]

ratio_8 =[122.420078,63.409120,27.024852,7.319500 ,3.693344]

ratio_16 =[108.153008,46.606660,23.360690,6.021432,3.184069]

ratio_32= [107.785520,45.707802,22.154515,5.748817,3.023997]

ratio_64 =[89.781482,42.886330,21.211577,5.525562,2.912102]


plt.plot(total_rank_number, ratio_64, color='green', linestyle='-', linewidth = 1, 
         marker='o', markerfacecolor='blue', markersize=3,label = "64 threads per MPI rank") 

plt.plot(total_rank_number, ratio_32, color='red', linestyle='-', linewidth = 1, 
         marker='o', markerfacecolor='y', markersize=3,label = "32 threads per MPI rank") 

plt.plot(total_rank_number, ratio_16, color='cyan', linestyle='-', linewidth = 1, 
         marker='o', markerfacecolor='coral', markersize=3,label = "16 threads per MPI rank") 

plt.plot(total_rank_number, ratio_8, color='purple', linestyle='-', linewidth = 1, 
         marker='o', markerfacecolor='gold', markersize=3,label = "8 threads per MPI rank") 

plt.plot(total_rank_number, ratio_4, color='black', linestyle='-', linewidth = 1, 
         marker='o', markerfacecolor='grey', markersize=3,label = "4 threads per MPI rank") 




plt.legend()


# naming the x axis 
plt.xlabel('Total Number of MPI Ranks * Threads per Rank ') 
# naming the y axis 
plt.ylabel('Total Time (seconds)') 
# giving a title to my graph 
plt.title('Total Time vs. Total Number of MPI Ranks * Threads per Rank') 
# function to show the plot 
plt.show()



####################################################################################################
##  Figure 2: Plot X:  Total Number of MPI Ranks * Threads per Rank  Y: speedup                   ##
####################################################################################################

#compute the speedup using the formula (the compute execution time of 8 threads
#per MPI rank using 4 compute nodes)/(the compute execution time of any one experiment).

time_4_nodes = [110.003835,122.420078,108.153008, 107.785520,89.781482]

time_8_nodes = [65.929427,63.409120,46.606660,45.707802,42.886330]

time_16_nodes = [35.787762,27.024852,23.360690,22.154515,21.211577]

time_64_nodes = [17.894622,7.319500 ,6.021432,5.748817,5.525562]

time_128_nodes = [9.466195,3.693344,3.184069,3.023997,2.912102]


mpi_rank_number2= [256, 512, 1024, 4096, 8192]

speedup2_ratio_4 =[]
speedup2_ratio_8 =[]
speedup2_ratio_16 =[]
speedup2_ratio_32 =[]
speedup2_ratio_64 =[]

speedup2_ratio_4.append(time_4_nodes[1]/time_4_nodes[0])
speedup2_ratio_4.append(time_4_nodes[1]/time_8_nodes[0])
speedup2_ratio_4.append(time_4_nodes[1]/time_16_nodes[0])
speedup2_ratio_4.append(time_4_nodes[1]/time_64_nodes[0])
speedup2_ratio_4.append(time_4_nodes[1]/time_128_nodes[0])

speedup2_ratio_8.append(time_4_nodes[1]/time_4_nodes[1])
speedup2_ratio_8.append(time_4_nodes[1]/time_8_nodes[1])
speedup2_ratio_8.append(time_4_nodes[1]/time_16_nodes[1])
speedup2_ratio_8.append(time_4_nodes[1]/time_64_nodes[1])
speedup2_ratio_8.append(time_4_nodes[1]/time_128_nodes[1])

speedup2_ratio_16.append(time_4_nodes[1]/time_4_nodes[2])
speedup2_ratio_16.append(time_4_nodes[1]/time_8_nodes[2])
speedup2_ratio_16.append(time_4_nodes[1]/time_16_nodes[2])
speedup2_ratio_16.append(time_4_nodes[1]/time_64_nodes[2])
speedup2_ratio_16.append(time_4_nodes[1]/time_128_nodes[2])

speedup2_ratio_32.append(time_4_nodes[1]/time_4_nodes[3])
speedup2_ratio_32.append(time_4_nodes[1]/time_8_nodes[3])
speedup2_ratio_32.append(time_4_nodes[1]/time_16_nodes[3])
speedup2_ratio_32.append(time_4_nodes[1]/time_64_nodes[3])
speedup2_ratio_32.append(time_4_nodes[1]/time_128_nodes[3])

speedup2_ratio_64.append(time_4_nodes[1]/time_4_nodes[4])
speedup2_ratio_64.append(time_4_nodes[1]/time_8_nodes[4])
speedup2_ratio_64.append(time_4_nodes[1]/time_16_nodes[4])
speedup2_ratio_64.append(time_4_nodes[1]/time_64_nodes[4])
speedup2_ratio_64.append(time_4_nodes[1]/time_128_nodes[4])

plt.plot(mpi_rank_number2, speedup2_ratio_64, color='green', linestyle='-', linewidth = 1, 
         marker='o', markerfacecolor='blue', markersize=3,label = "64 threads per MPI rank") 

plt.plot(mpi_rank_number2, speedup2_ratio_32, color='red', linestyle='-', linewidth = 1, 
         marker='o', markerfacecolor='y', markersize=3,label = "32 threads per MPI rank") 

plt.plot(mpi_rank_number2, speedup2_ratio_16, color='cyan', linestyle='-', linewidth = 1, 
         marker='o', markerfacecolor='coral', markersize=3,label = "16 threads per MPI rank") 

plt.plot(mpi_rank_number2, speedup2_ratio_8, color='purple', linestyle='-', linewidth = 1, 
         marker='o', markerfacecolor='gold', markersize=3,label = "8 threads per MPI rank") 

plt.plot(mpi_rank_number2, speedup2_ratio_4, color='black', linestyle='-', linewidth = 1, 
         marker='o', markerfacecolor='grey', markersize=3,label = "4 threads per MPI rank") 


plt.legend()


# naming the x axis 
plt.xlabel('Total Number of MPI Ranks * Threads per Rank') 
# naming the y axis 
plt.ylabel('Speedup') 
# giving a title to my graph 
plt.title('Speedup Relative to 8 Threads per MPI Rank for 4 Compute Nodes vs. Total Number of Processors ') 
# function to show the plot 
plt.show()

print ("Speedup")
print ("Ratio 4")
print (speedup2_ratio_4)
print ("Ratio 8")
print (speedup2_ratio_8)
print ("Ratio 16")
print (speedup2_ratio_16)
print ("Ratio 32")
print (speedup2_ratio_32)
print ("Ratio 64")
print (speedup2_ratio_64)

#############################################################################################################
##  Figure 3: Plot X:  Total Number of MPI Ranks * Threads per Rank  Y: Speedup/Total Number of Processors ##
#############################################################################################################

speedup3_ratio_4 =[]
speedup3_ratio_8 =[]
speedup3_ratio_16 =[]
speedup3_ratio_32 =[]
speedup3_ratio_64 =[]

speedup3_ratio_4.append(time_4_nodes[1]/time_4_nodes[0]/mpi_rank_number2[0])
speedup3_ratio_4.append(time_4_nodes[1]/time_8_nodes[0]/mpi_rank_number2[1])
speedup3_ratio_4.append(time_4_nodes[1]/time_16_nodes[0]/mpi_rank_number2[2])
speedup3_ratio_4.append(time_4_nodes[1]/time_64_nodes[0]/mpi_rank_number2[3])
speedup3_ratio_4.append(time_4_nodes[1]/time_128_nodes[0]/mpi_rank_number2[4])

speedup3_ratio_8.append(time_4_nodes[1]/time_4_nodes[1]/mpi_rank_number2[0])
speedup3_ratio_8.append(time_4_nodes[1]/time_8_nodes[1]/mpi_rank_number2[1])
speedup3_ratio_8.append(time_4_nodes[1]/time_16_nodes[1]/mpi_rank_number2[2])
speedup3_ratio_8.append(time_4_nodes[1]/time_64_nodes[1]/mpi_rank_number2[3])
speedup3_ratio_8.append(time_4_nodes[1]/time_128_nodes[1]/mpi_rank_number2[4])

speedup3_ratio_16.append(time_4_nodes[1]/time_4_nodes[2]/mpi_rank_number2[0])
speedup3_ratio_16.append(time_4_nodes[1]/time_8_nodes[2]/mpi_rank_number2[1])
speedup3_ratio_16.append(time_4_nodes[1]/time_16_nodes[2]/mpi_rank_number2[2])
speedup3_ratio_16.append(time_4_nodes[1]/time_64_nodes[2]/mpi_rank_number2[3])
speedup3_ratio_16.append(time_4_nodes[1]/time_128_nodes[2]/mpi_rank_number2[4])

speedup3_ratio_32.append(time_4_nodes[1]/time_4_nodes[3]/mpi_rank_number2[0])
speedup3_ratio_32.append(time_4_nodes[1]/time_8_nodes[3]/mpi_rank_number2[1])
speedup3_ratio_32.append(time_4_nodes[1]/time_16_nodes[3]/mpi_rank_number2[2])
speedup3_ratio_32.append(time_4_nodes[1]/time_64_nodes[3]/mpi_rank_number2[3])
speedup3_ratio_32.append(time_4_nodes[1]/time_128_nodes[3]/mpi_rank_number2[4])

speedup3_ratio_64.append(time_4_nodes[1]/time_4_nodes[4]/mpi_rank_number2[0])
speedup3_ratio_64.append(time_4_nodes[1]/time_8_nodes[4]/mpi_rank_number2[1])
speedup3_ratio_64.append(time_4_nodes[1]/time_16_nodes[4]/mpi_rank_number2[2])
speedup3_ratio_64.append(time_4_nodes[1]/time_64_nodes[4]/mpi_rank_number2[3])
speedup3_ratio_64.append(time_4_nodes[1]/time_128_nodes[4]/mpi_rank_number2[4])

plt.plot(mpi_rank_number2, speedup3_ratio_64, color='green', linestyle='-', linewidth = 1, 
         marker='o', markerfacecolor='blue', markersize=3,label = "64 threads per MPI rank") 

plt.plot(mpi_rank_number2, speedup3_ratio_32, color='red', linestyle='-', linewidth = 1, 
         marker='o', markerfacecolor='y', markersize=3,label = "32 threads per MPI rank") 

plt.plot(mpi_rank_number2, speedup3_ratio_16, color='cyan', linestyle='-', linewidth = 1, 
         marker='o', markerfacecolor='coral', markersize=3,label = "16 threads per MPI rank") 

plt.plot(mpi_rank_number2, speedup3_ratio_8, color='purple', linestyle='-', linewidth = 1, 
         marker='o', markerfacecolor='gold', markersize=3,label = "8 threads per MPI rank") 

plt.plot(mpi_rank_number2, speedup3_ratio_4, color='black', linestyle='-', linewidth = 1, 
         marker='o', markerfacecolor='grey', markersize=3,label = "4 threads per MPI rank") 



#plt.legend(loc='center left' , bbox_to_anchor=(1,0.5) )
plt.legend()


# naming the x axis 
plt.xlabel('Total Number of MPI Ranks * Threads per Rank') 
# naming the y axis 
plt.ylabel('Speedup/Total Number of Processors') 
# giving a title to my graph 
plt.title('Parallel Efficiency ') 
# function to show the plot 
plt.show()

print ("Parallel Efficiency")
print ("Ratio 4")
print (speedup3_ratio_4)
print ("Ratio 8")
print (speedup3_ratio_8)
print ("Ratio 16")
print (speedup3_ratio_16)
print ("Ratio 32")
print (speedup3_ratio_32)
print ("Ratio 64")
print (speedup3_ratio_64)

