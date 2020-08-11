# Project: Parallel Matrix Multiplication Algorithms

We implement two parallel matrix multiplication algorithms -- the Cannon's algorithm and the strip algorithm -- using MPI, Pthreads, and MPI I/O and conduct a performance study on the IBM Blue Gene/Q

Team Members: Jinghua Feng & Miao Qi 


## Member Contribution
### Jinghua Feng
1. Code implementation for Cannon's algorithm
2. Report: 
    - Implementation of Parallel Algorithms: contents related to Cannon's algorithm
    - Experimental Results and Discussion: contents related to Cannon's algorithm 
    - Comparision of two algorithms/Conclusion
### Miao Qi
1. Code implementation for the strip parallel algorithm
2. Report: 
    - Abstract/Introduction/Related Work/Future Work
    - Implementation of Parallel Algorithms: contents related to the strip parallel algorithm
    - Experimental Results and Discussion: contents related to the strip parallel algorithm

## Code
Code locates at BG/Q: "/gpfs/u/home/PCP8/PCP8fngj/scratch/matrix-multiplication". 

Folder details and How to run the codes:
### 1. cannon/
- what is it?
    
    c programming code for cannon algorithm, bash scripts to complie and run the c code, python scripts to plot figures, and ouput including text files for data and plots. 

- How to run code?
    - run the cannon algorithm (cannon_alg.c) on BG/Q: 
        ```bash
        $ ./cannon_alg_compile.sh
        $ ./cannon_alg_sbatch.sh num_of_nodes matrix_rows time_limit
        ```
    - plot Figures 3-10 in the report for total running time, speedup, parallel efficiency, and time distribution.
        ```bash
        $ ./cannon_alg_plot.py
        ```
    In addition, all text files for data obtained from BG/Q and corresponding figures are stored in sub-folder "output/" 

### 2. strip/
- what is it?
    
    c programming code for strip algorithm, bash scripts to complie and run the c code, and python scripts to plot figures
- How to run code?
    - run the strip algorithm (strip_alg.c) on BG/Q: 
            ```bash
            $ ./strip_alg_compile.sh
            $ ./strip_alg_sbatch.sh   num_of_nodes    thrds_per_rk    time_limit
            ```
    - plot Figures 11-13 in the report for total running time, overall speedup, and parallel efficiency.
            ```bash
            $ ./strip_alg_plot.py
            ```
    In addition, text files including Initial conditions of the experiment, Overhead time , Total execution time, and Real Compute execution time for all 25 experiments.  
