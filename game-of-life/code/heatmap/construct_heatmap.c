/*
 * Read data from binary file  (count: 32768 * 32768) via parallel I/O 
 * Set mpi_commsize = HEATMAP_SIZE
 * Set mpi_commsize = 1024 ( 16 nodes with 64 ranks in each node ), then each rank deals with 32 rows
 * Construct 1K X 1K heatmap with each element accumulating 32*32 grid of live/dead states 
 * Output obtained grid to file "heatmap.bin" 
 *
 */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<math.h>

#include<mpi.h>
#include<pthread.h>


/***************************************************************************/
/* Defines *****************************************************************/
/***************************************************************************/

#define ALIVE 1
#define DEAD  0
#define GRID_SIZE 32768 
// #define GRID_SIZE 32 
#define HEATMAP_CHUNK_SIZE 32
// #define HEATMAP_CHUNK_SIZE 4 
#define HEATMAP_SIZE GRID_SIZE/HEATMAP_CHUNK_SIZE 

/***************************************************************************/
/* Function: Main **********************************************************/
/***************************************************************************/

int main(int argc, char *argv[])
{
    int mpi_myrank;
    int mpi_commsize;
    int row_per_rank ;
    int chunk_int_num ;

    int nodes ;
    int threads_per_rank ;
    double random_threshold ;
    int ticks ; 

    nodes = atoi( argv[ 1 ] ) ;
    threads_per_rank = atoi( argv[ 2 ] ) ;
    random_threshold = atof( argv[ 3 ] ) ;
    ticks = atoi( argv[ 4 ] ) ;
    char universe_fname[ 80 ] ;
    char heatmap_fname[ 80 ] ;

    sprintf( universe_fname , "nds_%d_tpr_%d_thh_%f_tks_%d.bin" , 
             nodes , threads_per_rank , random_threshold , ticks) ;

    sprintf( heatmap_fname , "heatmap_nds_%d_tpr_%d_thh_%f_tks_%d.bin" , 
             nodes , threads_per_rank , random_threshold , ticks) ;

    // -------------------------------------------------------------------
    // Parameters for MPI I/O
    // -------------------------------------------------------------------
    int * r_buff ; // buffer for MPI_File_read_at
    int * hm_buff ; // buffer for writing heatmap 
    MPI_File fh;
    MPI_Offset offset ;
    MPI_Status status;

    // -------------------------------------------------------------------
    // MPI initialization 
    // -------------------------------------------------------------------
    MPI_Init( &argc, &argv) ;
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_commsize ) ;
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_myrank ) ;

    // Number of chunk array in each MPI rank
    chunk_int_num = GRID_SIZE * GRID_SIZE / mpi_commsize ;


    // ----------------------------------------------------------------------------------------------------
    // Read chunk array from file "universe.bin" to r_buff 
    // ----------------------------------------------------------------------------------------------------
    r_buff = ( int * ) malloc( chunk_int_num * sizeof( int ) ) ;
    MPI_File_open( MPI_COMM_WORLD, universe_fname , MPI_MODE_RDWR | MPI_MODE_CREATE , MPI_INFO_NULL, &fh ) ;
    offset = mpi_myrank * chunk_int_num * sizeof( int ) ;
    MPI_File_read_at( fh , offset , r_buff, chunk_int_num , MPI_INT , &status ) ;
    MPI_Barrier( MPI_COMM_WORLD );
    int i , j ;
    for( j = 0 ; j < mpi_commsize ; j++ ) {
        if( mpi_myrank == j ) {
            for( i = 0 ; i < chunk_int_num ; i++ ) {
                //printf( "%d " , r_buff[ i ] ) ;
            }
        }
    }

    MPI_Barrier( MPI_COMM_WORLD );
    MPI_File_close( &fh ); 

    // Number of rows in each MPI rank 
    row_per_rank = GRID_SIZE / mpi_commsize ;

    if( row_per_rank != HEATMAP_CHUNK_SIZE ){
        perror( "Wrong mpi_commsize, mpi_commsize = GRID_SIZE / HEAT_MAP_SIZE" ) ;
        exit( EXIT_FAILURE ) ;
    }


    //--------------------------------------------------------------------
    // Allocate and initialize 1D heatmap buff
    // Each rank contributes to one row of final heatmap
    //--------------------------------------------------------------------
    hm_buff = ( int * ) malloc( HEATMAP_SIZE * sizeof( int ) ) ;
    for( i = 0 ; i < HEATMAP_SIZE ; i++ ){
        hm_buff[ i ] = 0 ;
    }
    
    //--------------------------------------------------------------------
    // Computing heatmap buff based on data read from "universe.bin" 
    //--------------------------------------------------------------------
    int k ;
    for( i = 0 ; i < chunk_int_num ;  i++ ){

        // j - col index if converting r_buff to 2D array
        j = i % GRID_SIZE ;

        // k - index of hm_buff that r_buff[ i ] should be accumulated to 
        k = j / HEATMAP_CHUNK_SIZE ;

        hm_buff[ k ] += r_buff[ i ] ;
    }

    /*
    for( i = 0 ; i < HEATMAP_SIZE ; i++ ){
        printf( "%d " , hm_buff[ i ] ) ;
    }
    */

    //--------------------------------------------------------------------
    // Write data in hm_buff to file "heatmap.bin" 
    //--------------------------------------------------------------------
    MPI_File_open( MPI_COMM_WORLD, heatmap_fname , MPI_MODE_RDWR | MPI_MODE_CREATE , MPI_INFO_NULL, &fh ) ;
    offset = mpi_myrank * HEATMAP_SIZE * sizeof( int ) ;
    //MPI_File_write_at( fh , offset , hm_buff , chunk_int_num , MPI_INT, &status );
    MPI_File_write_at( fh , offset , hm_buff ,  HEATMAP_SIZE , MPI_INT, &status );

    MPI_Barrier( MPI_COMM_WORLD );
    MPI_File_close( &fh ); 
    

    //--------------------------------------------------------------------
    // END -Perform a barrier and then leave MPI
    // Free dynamic memory
    //--------------------------------------------------------------------
    MPI_Barrier( MPI_COMM_WORLD );
    free( r_buff ) ;
    free( hm_buff ) ;

    MPI_Finalize();
    return 0;
}
