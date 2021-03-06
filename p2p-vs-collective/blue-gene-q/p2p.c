/*
 * Goal: sum all entries of array [0,1,..., 2^30-1] via the two approaches below
 *      (1) Collective approach: MPI_reduce()
 *      (2) P2P: point-2-point message version MPI_P2P_reduce() of MPI_reduce()
 * Compare the performance of MPI_P2P_reduce wth the collective version MPI_reduce across a variable number of MPI rank configurations
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mpi.h>

#define BGQ 1 // when running BG/Q, comment out when running on mastiff

#ifdef BGQ
#include<hwi/include/bqc/A2_inlines.h>
unsigned long long start_cycles = 0;
unsigned long long end_cycles = 0;
double processor_frequency = 1600000000.0;

#else
double start_cycles = 0;
double end_cycles = 0;
double processor_frequency = 1.0;
#define GetTimeBase MPI_Wtime
#endif

// #define INPUT_SIZE 1073741824
#define MY_INPUT_SIZE 1024 * 1024 *1024
int MY_MPI_RANK = -1 ;
int MY_MPI_SIZE = -1 ;

double time_in_secs = 0;

unsigned long long CollectiveReduce( unsigned long long chunk_size ) ;
int PointToPointReduce( const unsigned long long chunk_size , unsigned long long * final_sum ) ;

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
    if( MY_MPI_RANK == 0 ){
        // print for submitty
        printf( "%llu %f\n" , collect_final_sum , time_in_secs ) ;

        // uncomment "/* */" if printing detailed execution
        /*
        printf( "collect_final_sum: %llu \n" , collect_final_sum ) ;
        change "%llu" to "%f" if defined as double
        printf( "start_cycles: %llu \n" , start_cycles ) ;
        printf( "end_cycles: %llu \n" , end_cycles ) ;
        printf( "time_in_secs: %f \n\n" , time_in_secs ) ;
	    */
        
        // uncomment "//" if only execution needed
        // printf( "%f\t" , time_in_secs ) ;
    }

    /******************** Point2Point ***********************/
    start_cycles= GetTimeBase();
    PointToPointReduce( chunk_size , & P2P_final_sum ) ;
    end_cycles= GetTimeBase();
    time_in_secs = ((double)(end_cycles - start_cycles)) / processor_frequency;
    if( MY_MPI_RANK == 0 ) { 

        // print for submitty
        printf( "%llu %f\n" , P2P_final_sum , time_in_secs ) ;

        // uncomment "/* */" if printing detailed execution
        /*
        printf( "P2P_final_sum: %llu \n" , P2P_final_sum ) ;
        // change "%llu" to "%f" if defined as double
        printf( "start_cycles: %llu \n" , start_cycles ) ;
        printf( "end_cycles: %llu \n" , end_cycles ) ;
        printf( "time_in_secs: %f \n" , time_in_secs ) ;
        */

        // uncomment "//" if only execution needed
        // printf( "%f\n" , time_in_secs ) ;
    }

    MPI_Finalize() ;
    return EXIT_SUCCESS ;
}


// Collective approach by using MPI_reduce()
unsigned long long CollectiveReduce( const unsigned long long chunk_size ) {
    unsigned long long local_sum = 0 ;
    unsigned long long final_sum , i ;

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
    int max_layer = ( int ) ( log10( ( double ) MY_MPI_SIZE ) / log10( 2.0 ) ) ;

    MPI_Request isend_request , irecv_request ;
    MPI_Status irecv_status ;

    // l: layer iter 
    // j, j-tmp: pairwise ranks at each layer to execture MPI_Isend and MPI_Irecv respectively
    int l , j , tmp ;
    for( l = 0 ; l < max_layer ; l++ ) {
        // j_ini = 2^l , j_fin = MY_MPI_SIZE - 2^l , j_del = 2^(l+1)
        tmp = ( int ) pow( 2.0 , ( double ) l ) ;
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
