/***************************************************************************/
/* Template for Asssignment 4/5 ********************************************/
/* Team Names Here              **(*****************************************/
/***************************************************************************/

/***************************************************************************/
/* Includes ****************************************************************/
/***************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<math.h>

#include<clcg4.h>

#include<mpi.h>
#include<pthread.h>

// #define BGQ 1 // when running BG/Q, comment out when testing on mastiff

#ifdef BGQ
#include<hwi/include/bqc/A2_inlines.h>
#else
#define GetTimeBase MPI_Wtime            
#endif

/***************************************************************************/
/* Defines *****************************************************************/
/***************************************************************************/

#define ALIVE 1
#define DEAD  0

//#define ROWS 1024
#define ROWS 4 

/***************************************************************************/
/* Global Vars *************************************************************/
/***************************************************************************/
// const int ROWS = 1024 ;

double g_time_in_secs = 0;
double g_processor_frequency = 1600000000.0; // processing speed for BG/Q
unsigned long long g_start_cycles=0;
unsigned long long g_end_cycles=0; // You define these 
/***************************************************************************/
/* Function Decs ***********************************************************/
/***************************************************************************/

// You define these


/***************************************************************************/
/* Function: Main **********************************************************/
/***************************************************************************/

int main(int argc, char *argv[])
{
    int mpi_myrank;
    int mpi_commsize;
    int * buff ;
    int * buff2 ;

    MPI_File fh;
    MPI_Offset offset ;
    MPI_Offset file_size ;
    MPI_Status status;
    int chunk_int_num ;


// Example MPI startup and using CLCG4 RNG
    MPI_Init( &argc, &argv) ;
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_commsize ) ;
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_myrank ) ;
    
// Init 32,768 RNG streams - each rank has an independent stream
    InitDefault();
    
// Note, used the mpi_myrank to select which RNG stream to use.
// You must replace mpi_myrank with the right row being used.
// This just show you how to call the RNG.    
    printf("Rank %d of %d has been started and a first Random Value of %lf\n", 
	   mpi_myrank, mpi_commsize, GenVal(mpi_myrank));

    chunk_int_num = ROWS * ROWS / mpi_commsize ;
    buff = ( int * ) malloc( chunk_int_num * sizeof( int ) ) ;
    int i ;
    for( i = 0 ; i < chunk_int_num ; i++ ) {
        buff[ i ] = mpi_myrank * chunk_int_num + i ;
        printf( "%d " , buff[ i ] ) ;
    }

    offset = mpi_myrank * chunk_int_num * sizeof( int ) ;
    //printf( "offset of rank %d: %d" , mpi_myrank , offset ) ;
    //offset = mpi_myrank * chunk_int_num ;
    // MPI_File_open( MPI_COMM_WORLD, "universe.txt", MPI_MODE_CREATE | MPI_MODE_WRONLY , MPI_INFO_NULL, &fh ) ;
    MPI_File_open( MPI_COMM_WORLD, "universe.txt", MPI_MODE_RDWR | MPI_MODE_CREATE , MPI_INFO_NULL, &fh ) ;
    // MPI_File_write_at( fh , offset , buff , chunk_int_num * sizeof( int ) , MPI_INT, &status );
    MPI_File_write_at( fh , offset , buff , chunk_int_num , MPI_INT, &status );
    MPI_Barrier( MPI_COMM_WORLD );

    buff2 = ( int * ) malloc( ROWS * ROWS * sizeof( int ) ) ;
    
    MPI_Barrier( MPI_COMM_WORLD );
    MPI_File_read_at(fh, offset, buff2, chunk_int_num , MPI_INT , &status) ;
    MPI_Barrier( MPI_COMM_WORLD );
    if( mpi_myrank == 0 ){
        printf( "Read to buff2!\n") ;
        for( i = 0 ; i < ROWS * ROWS ; i++ ) {
            printf( "%d " , buff2[ i ] ) ;
        }
    }
    
    /*
    MPI_Barrier( MPI_COMM_WORLD );
    if( mpi_myrank == 0 ){
        MPI_File_get_size( fh , & file_size ) ;
        printf( "file_size: %ll" , file_size ) ;
    }
    */

    MPI_File_close( &fh ); 


// END -Perform a barrier and then leave MPI
    MPI_Finalize();
    free( buff ) ;
    free( buff2 ) ;

    return 0;
}

/***************************************************************************/
/* Other Functions - You write as part of the assignment********************/
/***************************************************************************/
