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
#define GRID_SIZE 16 

/***************************************************************************/
/* Global Vars *************************************************************/
/***************************************************************************/

double g_time_in_secs = 0;
double g_processor_frequency = 1600000000.0; // processing speed for BG/Q
unsigned long long g_start_cycles=0;
unsigned long long g_end_cycles=0; // You define these 
/***************************************************************************/
/* Function Decs ***********************************************************/
/***************************************************************************/
int** Allocation(int input_rows, int input_cols) ;
void Initialization(int **input_grid_chunk, int input_rows, int input_cols) ;
void PrintGrid(int **input_grid_chunk, int start_row, int end_row) ;
void PrintGridAddress(int **input_grid_chunk, int start_row, int end_row) ;


// You define these


/***************************************************************************/
/* Function: Main **********************************************************/
/***************************************************************************/

int main(int argc, char *argv[])
{
    int mpi_myrank;
    int mpi_commsize;

    //-------- Parameters for MPI I/O -----------------
    int * w_buff ; // buffer for MPI_File_write_at
    int * r_buff ; // buffer for MPI_File_read_at
    MPI_File fh;
    MPI_Offset offset ;
    MPI_Status status;

    // Number of chunk array in each MPI rank
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

    chunk_int_num = GRID_SIZE * GRID_SIZE / mpi_commsize ;

    int **local_universe = Allocation ( GRID_SIZE , GRID_SIZE );
    Initialization(local_universe, GRID_SIZE , GRID_SIZE );

    /*
    // Allocate memory for chunk array, 
    w_buff = ( int * ) malloc( chunk_int_num * sizeof( int ) ) ;

    // Assign arbitrary values to chunk array, should be replaced by real universe array untimately 
    int i ;
    for( i = 0 ; i < chunk_int_num ; i++ ) {
        w_buff[ i ] = mpi_myrank * chunk_int_num + i ;
    }
    */

    // ----------------------------------------------------------------------------------------------------
    // Write chunk array from buffer to file "universe.bin"
    // ----------------------------------------------------------------------------------------------------
    offset = mpi_myrank * chunk_int_num * sizeof( int ) ;
    int row_per_rank = GRID_SIZE / mpi_commsize ;
    MPI_File_open( MPI_COMM_WORLD, "universe.bin", MPI_MODE_RDWR | MPI_MODE_CREATE , MPI_INFO_NULL, &fh ) ;
    int i ;
    for( i = 0 ; i < row_per_rank ; i++ ) {
        MPI_File_write_at( fh , offset , local_universe[ i ] , GRID_SIZE , MPI_INT, &status );
        offset += GRID_SIZE * sizeof( int ) ;
    }

    // Write 1-d array
    //MPI_File_write_at( fh , offset , w_buff , chunk_int_num , MPI_INT, &status );

    MPI_Barrier( MPI_COMM_WORLD );

    // ----------------------------------------------------------------------------------------------------
    // Read chunk array from file to buffer 
    // ----------------------------------------------------------------------------------------------------
    r_buff = ( int * ) malloc( chunk_int_num * sizeof( int ) ) ;
    offset = mpi_myrank * chunk_int_num * sizeof( int ) ;
    MPI_File_read_at( fh , offset , r_buff, chunk_int_num , MPI_INT , &status ) ;
    MPI_Barrier( MPI_COMM_WORLD );
    int j ;
    for( j = 0 ; j < mpi_commsize ; j++ ) {
        if( mpi_myrank == j ) {
            for( i = 0 ; i < chunk_int_num ; i++ ) {
                printf( "%d " , r_buff[ i ] ) ;
            }
        }
    }

    MPI_File_close( &fh ); 

    // END -Perform a barrier and then leave MPI
    MPI_Finalize();
    //free( w_buff ) ;
    free( r_buff ) ;

    return 0;
}

/***************************************************************************/
/* Other Functions - You write as part of the assignment********************/
/***************************************************************************/

int** Allocation(int input_rows, int input_cols)
{
    int **input_grid = (int **) malloc( input_rows *sizeof(int*));
    int row_index = 0;
    for (; row_index < input_rows; ++row_index) 
    {
        input_grid[row_index] = (int*)malloc( input_cols * sizeof(int) );
    }
    return input_grid;
}

void Initialization(int **input_grid_chunk, int input_rows, int input_cols)
{
    int i, j;
    for(i = 0; i < input_rows; i++)
    {
        for(j = 0; j < input_cols; j++)
        {
            input_grid_chunk[i][j] = (int)ALIVE;
        }
    }
}

void PrintGrid(int **input_grid_chunk, int start_row, int end_row)
{ 
    int i, j;
    for(i = start_row; i < end_row+1; i++)
    {
        for(j = 0; j < GRID_SIZE; j++)
        {
            printf("%d ",input_grid_chunk[i][j]);
        }
        printf("\n");
    }

}

void PrintGridAddress(int **input_grid_chunk, int start_row, int end_row)
{ 
    int i, j;
    for(i = start_row; i < end_row+1; i++)
    {
        for(j = 0; j < GRID_SIZE; j++)
        {
            printf("%d ",& input_grid_chunk[i][j]);
        }
        printf("\n");
    }

}

void FreeGrid(int **input_grid_chunk, int input_rows)
{
    int i;
    for(i = 0; i < input_rows; i++)
    {
        free(input_grid_chunk[i]);
    }
    free(input_grid_chunk);
}
