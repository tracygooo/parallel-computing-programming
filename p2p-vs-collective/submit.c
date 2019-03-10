/*
 * Goal: sum all entries of array [0,1,..., 2^30-1] via the two approaches below
 *      (1) Collective approach: MPI_reduce()
 *      (2) P2P: point-2-point message version MPI_P2P_reduce() of MPI_reduce()
 * Compare the performance of MPI_P2P_reduce wth the collective version MPI_reduce across a variable number of MPI rank configurations
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

// For running BG/Q, comment out when running on mastiff
// #define BGQ 1 

// Use GetTimeBase() to compute running time on BG/Q
#ifdef BGQ
#include<hwi/include/bqc/A2_inlines.h>
unsigned long long start_cycles = 0;
unsigned long long end_cycles = 0;
double processor_frequency = 1600000000.0;

// Use MPI_Wtime() to compute running time on Mastiff 
#else
double start_cycles = 0;
double end_cycles = 0;
double processor_frequency = 1.0;
#define GetTimeBase MPI_Wtime
#endif

// Global variable for size of the single long array 
#define MY_INPUT_SIZE 1024 * 1024 *1024

// Global variables for MPI rank and size
int MY_MPI_RANK = -1 ;
int MY_MPI_SIZE = -1 ;

// Global variable for running time 
double time_in_secs = 0;

// Declaration of functions 
unsigned long long CollectiveReduce( unsigned long long chunk_size ) ;
int PointToPointReduce( const unsigned long long chunk_size , unsigned long long * final_sum ) ;
int ComputePower( const int base , const int exponent ) ;
int ComputeLog2( const int power ) ;
int WriteOutput( const char * fname , const unsigned long long collect_sum , const unsigned long long p2p_sum ) ;

int main( int argc , char ** argv ) {

    /******************** MPI initialization ***********************/
    MPI_Init( & argc, & argv);
    MPI_Comm_size( MPI_COMM_WORLD, & MY_MPI_SIZE ) ;
    MPI_Comm_rank( MPI_COMM_WORLD, & MY_MPI_RANK ) ;

    unsigned long long chunk_size = MY_INPUT_SIZE / MY_MPI_SIZE ;
    unsigned long long collect_final_sum , P2P_final_sum ;

    /******************** Collective MPI_reduce ***********************/
    start_cycles= GetTimeBase();
    collect_final_sum = CollectiveReduce( chunk_size ) ;
    end_cycles= GetTimeBase();
    time_in_secs = ((double)(end_cycles - start_cycles)) / processor_frequency;

    /******************** Point2Point ***********************/
    start_cycles= GetTimeBase();
    PointToPointReduce( chunk_size , & P2P_final_sum ) ;
    end_cycles= GetTimeBase();
    time_in_secs = ((double)(end_cycles - start_cycles)) / processor_frequency;

    /******************** Output results ************************/
    if( MY_MPI_RANK == 0 ) { 
        //WriteOutput( "STDOUT_1.txt" , collect_final_sum , P2P_final_sum ) ;
        printf( "%llu\n%llu\n" , collect_final_sum , P2P_final_sum ) ;
    }

    MPI_Finalize() ;

    return EXIT_SUCCESS ;
}


// Collective approach by using MPI_reduce()
unsigned long long CollectiveReduce( const unsigned long long chunk_size ) {
    unsigned long long local_sum = 0 ;
    unsigned long long final_sum = 0 ;
    unsigned long long i ;

    // compute sum over local data array
    for( i = chunk_size * MY_MPI_RANK ; i < chunk_size * ( MY_MPI_RANK + 1 ) ; i++  )
        local_sum += i ;

    // conduct collective sum at root 0
    MPI_Reduce( & local_sum , & final_sum , 1 , MPI_UNSIGNED_LONG_LONG , MPI_SUM , 0 , MPI_COMM_WORLD ) ;

    return final_sum ;
}

// Develop Point2Point approach by using "binary tree structure" 
int PointToPointReduce( const unsigned long long chunk_size , unsigned long long * final_sum ) {
    unsigned long long local_sum = 0 ;
    unsigned long long pairwise_sum , i ;

    // compute sum over local data array
    for( i = chunk_size * MY_MPI_RANK ; i < chunk_size * ( MY_MPI_RANK + 1 ) ; i++  )
        local_sum += i ;

    // # of layers to obtain final sum 
    // int max_layer = ( int ) ( log10( ( double ) MY_MPI_SIZE ) / log10( 2.0 ) ) ;
    int max_layer =  ComputeLog2( MY_MPI_SIZE ) ;

    MPI_Request isend_request , irecv_request ;
    MPI_Status irecv_status ;

    /* 
     * l: layer iter 
     * j, j-tmp: pairwise ranks at each layer to execute MPI_Isend and MPI_Irecv respectively
     *
     */
    int l , j , tmp ;
    for( l = 0 ; l < max_layer ; l++ ) {
        // j_ini = 2^l , j_fin = MY_MPI_SIZE - 2^l , j_del = 2^(l+1)
        tmp = ComputePower( 2 , l ) ; 
        for( j = tmp ; j <= MY_MPI_SIZE - tmp ; j += 2*tmp ) {

            if( MY_MPI_RANK == j ) {
                MPI_Isend( & local_sum , 1 , MPI_UNSIGNED_LONG_LONG , j - tmp , 0 , MPI_COMM_WORLD , & isend_request ) ;
                return EXIT_SUCCESS ;
            }

            if( MY_MPI_RANK == j - tmp ) {
                MPI_Irecv( & pairwise_sum , 1 , MPI_UNSIGNED_LONG_LONG , j , 0 , MPI_COMM_WORLD , & irecv_request ) ;
                MPI_Wait( & irecv_request , & irecv_status ) ;
                local_sum += pairwise_sum ;
            }
        }
    }

    // After executing above loops, local_sum at rank 0 (root) is the final sum needed
    if( MY_MPI_RANK == 0 )
        * final_sum = local_sum ;

    return EXIT_SUCCESS ;
}

int ComputePower( const int base , const int exponent ) {
    int i ;
    int power = 1 ; 
    for( i = 0 ; i < exponent ; i ++ ) {
        power *= base ;
    }
    return power ;
}

// Compute logrithm with base 2
int ComputeLog2( const int power ) {
    int exponent = 0 ;
    int tmp = 1 ;
    while( tmp < power ){
        tmp *= 2 ;
        exponent += 1 ;
    }
    return exponent ;
}

// Write sums to file
int WriteOutput( const char * fname , const unsigned long long collect_sum , const unsigned long long p2p_sum ) { 

    FILE * file ;
    if( ( file = fopen( fname , "w" ) ) == NULL ) {
            printf( "Failed to open output data file: %s\n" , fname ) ;
            exit( EXIT_FAILURE ) ;
        }
    fprintf( file , "%llu\n%llu\n" , collect_sum , p2p_sum ) ;
    fclose( file ) ;

    return EXIT_SUCCESS ;
}
