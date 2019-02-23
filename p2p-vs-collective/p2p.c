/*
 * Goal: sum array [0,1,..., 2^30-1] 
 * Develop a point-2-point message version MPI_P2P_reduce of the MPI reduce operation
 * Compare the performance of MPI_P2P_reduce wth the collective version MPI_reduce across a variable number of MPI rank configurations
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

// #define BGQ 1 // when running BG/Q, comment out when running on mastiff
#ifdef BGQ
#include<hwi/include/bqc/A2_inlines.h>
#else
#define GetTimeBase MPI_Wtime
#endif

// #define INPUT_SIZE 1073741824
#define MY_INPUT_SIZE 100
int MY_MPI_RANK = -1 ;
int MY_MPI_SIZE = -1 ;

double time_in_secs = 0;
double processor_frequency = 1600000000.0;
unsigned long long start_cycles=0;
unsigned long long end_cycles=0;

int main( int argc , char ** argv ) {

    /*
    start_cycles= GetTimeBase();
    MPI_P2P_Reduce(.....);
    end_cycles= GetTimeBase();
    time_in_secs = ((double)(end_cycles - start_cycles)) / processor_frequency;
    */

    unsigned long long local_sum , final_sum ;

    MPI_Init( & argc, & argv);
    MPI_Comm_size( MPI_COMM_WORLD, & MY_MPI_SIZE ) ;
    MPI_Comm_rank( MPI_COMM_WORLD, & MY_MPI_RANK ) ;

    unsigned long long chunk_size = MY_INPUT_SIZE / MY_MPI_SIZE ;

    unsigned long long i ;
    local_sum = 0 ;
    for( i = chunk_size * MY_MPI_RANK ; i < chunk_size * ( MY_MPI_RANK + 1 ) ; i++  )
        local_sum += i ;

    MPI_Reduce( & local_sum , & final_sum , 1 , MPI_UNSIGNED_LONG_LONG , MPI_SUM , 0 , MPI_COMM_WORLD ) ;

    if( MY_MPI_RANK == 0 )
        printf( "final_sum:%llu \n" , final_sum ) ;

    MPI_Finalize() ;
    return EXIT_SUCCESS ;
}
