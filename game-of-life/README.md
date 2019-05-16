Contributors: 
    Jinghua Feng, fengj3@rpi.edu
    Miao Qi, qim@rpi.edu

We create an MPI implementation of Conway’s Game of Life to leverage Pthreads within compute nodes and conduct a performance study on the IBM Blue Gene/Q

There for three subfolders in folder "code", which are explained in details as below. 

1. ./code/play-game
    what is it? 
        c programming code for playing Conway's game of life, Makefile for compiling c code
    How to run code?
        (1) $ make
        (2) $ mpicc -np num-of-processors ./main threads_per_rank threshold num_ticks

2. ./code/heat-map
    what is it?
        c programming code and gnuplot script for constructing heatmap binary file, converting binary file to text file, and drawing heatmap 
    How to run code?
        (1) constructing heatmap (run in BG/Q): 
                $ ./compile_construct.sh
                $ ./sbatch_construct.sh
        (2) converting binary to text file (run in BG/Q): 
                $ ./compile_convert 
                $ ./sbatch_convert.sh
        (3) draw heatmap(run in desktop): 
                $ ./run_draw_heatmap                

3. ./code/plots
    what is it?
        python scripts to plot figures
    How to run code?
        (1) plot Figures 1-6 in the report for alive cells counts, execution time, etc.
            $ ./count_cells_execution.py
        (2) plot Figure 7 in the report for io time 
            $ ./io.py
