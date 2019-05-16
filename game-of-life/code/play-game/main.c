/*
 * Asssignment 4/5
 * Miao Qi, qim@rpi.edu 
 * Jinghua Feng, fengj3@rpi.edu 
 */


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

#define BGQ 1 // when running BG/Q, comment out when testing on mastiff

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

#define GRID_SIZE 32768 
//#define GRID_SIZE 1024 

/***************************************************************************/
/* Global Vars *************************************************************/
/***************************************************************************/

// Vars for computing execution time 
double g_time_in_secs = 0;
double g_processor_frequency = 1600000000.0; // processing speed for BG/Q
unsigned long long g_start_cycles=0;
unsigned long long g_end_cycles=0;

// Vars for computing parallel I/O time 
double g_time_in_secs_io = 0;
double g_processor_frequency_io = 1600000000.0; // processing speed for BG/Q
unsigned long long g_start_cycles_io=0;
unsigned long long g_end_cycles_io=0;

// MPI and pthread variables
int MY_MPI_RANK ;
int MY_MPI_SIZE ;
pthread_barrier_t THRD_BARRIER ; 

int ** LOCAL_UNIVERSE ;
int * ALIVE_CNT_LOC ;
int * ALIVE_CNT_GOL ;

int ROWS_PER_RANK ; // number of rows processed by each rank
int NUM_NODES ; // number of nodes used in BG/Q
int TOT_TICKS = 256 ; // number of ticks
double THRESHOLD = 0.25 ; // randomization threshold
int THRD_PER_RANK = 1 ; // number of threads per rank
int THRD_CREATED ; // THRD_PER_RANK - 1
char MY_FNAME[ 100 ] ; // Name of file for writing cell status by MPI I/O

/***************************************************************************/
/* Function Decs ***********************************************************/
/***************************************************************************/
int** Allocation(int input_rows, int input_cols);
void Initialization(int **input_grid_chunk, int input_rows, int input_cols);
void InitializeArray( int * my_array , const int arr_size ) ;
void InitializeLocalChunk( int ** mat_chunk , int * vec_chunk , const int vec_chunk_size ) ;

void * PlayGame( void * arg ) ;
int CountAliveCellsOnGrid( int ** grid_chunk , int start_row , int end_row ) ;
void UpdateCellWithBasicRules(int **input_grid_chunk, int start_row, int end_row) ;

void SendGhost( int * first_row , int * last_row ) ;
void RecvGhost( int * recv_pre , int * recv_next );
void MyReduce( int * loc_arr , int * glob_arr ) ;

void FreeGrid(int **input_grid_chunk, int input_rows) ;
void PrintIntArray( int* my_arr , const int arr_size , const char * arr_name ) ; 
void PrintGrid( int **input_grid_chunk, int start_row, int end_row) ;
void PrintInitConditions() ;
void SetParameters( char ** arg_vec ) ;  

/***************************************************************************/
/* Function: Main **********************************************************/
/***************************************************************************/

int main(int argc, char *argv[])
{


    // -----------------------------------------------------------------
    // Define parameters for MPI I/O
    // -----------------------------------------------------------------
    MPI_File fh;
    MPI_Offset offset ;
    MPI_Status status;
    int chunk_int_num ; // Number of cells in each MPI rank
    int * vec_chunk ;
   // int * r_buff ; // buffer for MPI_File_read_at

    // -----------------------------------------------------------------
    // initialization
    // -----------------------------------------------------------------
    // Init MPI
    MPI_Init( &argc, &argv);
    MPI_Comm_size( MPI_COMM_WORLD, & MY_MPI_SIZE );
    MPI_Comm_rank( MPI_COMM_WORLD, & MY_MPI_RANK );

    // Init parameters, ./main thread_per_rank threshold num_ticks 
    SetParameters( argv ) ;

    // if rank 0 / thread 0, start time with GetTimeBase()for execution time.
    if (0 == MY_MPI_RANK )
    {

        PrintInitConditions() ;
        g_start_cycles = GetTimeBase();
    }

    // Init global variables 
    ROWS_PER_RANK = GRID_SIZE / MY_MPI_SIZE ;
    THRD_CREATED = THRD_PER_RANK - 1 ; // As MPI master works as one thread

    // Init 32,768 RNG streams - each rank has an independent stream
    InitDefault();

    // Init LOCAL_UNIVERSE, +2 due to two ghost rows in each rank
    LOCAL_UNIVERSE = Allocation( ROWS_PER_RANK + 2 , GRID_SIZE ) ;
    Initialization( LOCAL_UNIVERSE , ROWS_PER_RANK + 2 , GRID_SIZE ) ;

    // Init array for total number of alive cells at each tick
    ALIVE_CNT_LOC = ( int * ) malloc( sizeof( int ) * TOT_TICKS ) ;
    InitializeArray( ALIVE_CNT_LOC , TOT_TICKS ) ;

    MPI_Barrier( MPI_COMM_WORLD );

    // Init thread barrier
    pthread_barrier_init( & THRD_BARRIER , NULL , THRD_PER_RANK ) ;


    // -----------------------------------------------------------------
    // Create Threads and threads play game 
    // -----------------------------------------------------------------
    int i ;
    int * tid = NULL ;
    pthread_t *p_threads = NULL ;
    if( THRD_CREATED != 0 ) {
        p_threads = ( pthread_t * ) malloc( THRD_CREATED * sizeof( pthread_t ) );
        pthread_attr_t attr; 
        pthread_attr_init (&attr);
        tid = ( int * ) malloc( sizeof( int) *THRD_CREATED ) ; 
        for( i = 1 ; i <= THRD_CREATED ; i++ ) {
            tid[ i - 1 ] = i ;
            if( pthread_create( &p_threads[ i - 1 ], &attr, PlayGame , & tid[ i - 1 ]  ) != 0 ){  
                fprintf( stderr, "MAIN: Could not create thread (%d)\n", i );
                return EXIT_FAILURE;
            }
        }
    }


    // -----------------------------------------------------------------
    // MPI master thread play game 
    // -----------------------------------------------------------------
    int master_id = 0 ;
    PlayGame( & master_id ) ; 

    //Clean the threads
    if( THRD_CREATED != 0 ) {
        for ( i = 0 ; i < THRD_CREATED ; i++)
        {
            pthread_join( p_threads[ i ] , NULL);

        }
    }

    // -----------------------------------------------------------------
    // Destroy pthread barrier 
    // -----------------------------------------------------------------
    if( pthread_barrier_destroy( & THRD_BARRIER ) != 0 ) {
        printf( "Failed to destroy pthread barrier in main()\n" ) ;
        exit( EXIT_FAILURE ) ;
    }


    // -----------------------------------------------------------------
    // Obtain count of all alive cells across ranks via MPI_Reduce
    // -----------------------------------------------------------------
    ALIVE_CNT_GOL = ( int * ) malloc( sizeof( int ) * TOT_TICKS ) ;
    MyReduce( ALIVE_CNT_LOC , ALIVE_CNT_GOL ) ;

    //if rank 0 / thread 0, end time with GetTimeBase() for execution time.
    MPI_Barrier( MPI_COMM_WORLD );
    if (0 == MY_MPI_RANK )
    {
        g_end_cycles = GetTimeBase() ;
        g_time_in_secs = ((double)( g_end_cycles - g_start_cycles ) ) / g_processor_frequency;
        printf("Compute execution time =  %f \n", g_time_in_secs);
    }

    // -----------------------------------------------------------------
    // Write chunk array from LOCAL_UNIVERSE to file "MY_FANME.bin"
    // -----------------------------------------------------------------
    chunk_int_num = GRID_SIZE * GRID_SIZE / MY_MPI_SIZE ;
    offset = MY_MPI_RANK * chunk_int_num * sizeof( int ) ;


    // Convert 2D LOCAL_UNIVERSE to 1D array
    vec_chunk = ( int * ) malloc( sizeof( int ) * chunk_int_num ) ;
    InitializeLocalChunk( LOCAL_UNIVERSE , vec_chunk , chunk_int_num ) ;

	// if rank 0 , start time with GetTimeBase() for I/O time.
    MPI_Barrier( MPI_COMM_WORLD );
	if (0 == MY_MPI_RANK )
        g_start_cycles_io = GetTimeBase();

    MPI_File_open( MPI_COMM_WORLD, MY_FNAME , MPI_MODE_RDWR | MPI_MODE_CREATE , MPI_INFO_NULL, &fh ) ;
    MPI_File_write_at( fh , offset , vec_chunk , chunk_int_num , MPI_INT, &status ) ;
    MPI_Barrier( MPI_COMM_WORLD );
	
	//if rank 0, print ALIVE tick stats and compute (I/O if needed) performance stats.
	if (0 == MY_MPI_RANK )
    {
        g_end_cycles_io = GetTimeBase();
		g_time_in_secs_io = ((double)(g_end_cycles_io - g_start_cycles_io))/g_processor_frequency_io;
		PrintIntArray( ALIVE_CNT_GOL , TOT_TICKS , "global count for ALIVE cells" ) ;
		printf("Parallel I/O time =  %f \n", g_time_in_secs_io);
	}


    // -----------------------------------------------------------------
    // Free dynamic memories
    // END -Perform a barrier and then leave MPI
    // -----------------------------------------------------------------
    FreeGrid( LOCAL_UNIVERSE, ROWS_PER_RANK + 2 );
    if( THRD_CREATED != 0 ) { 
        free(p_threads);
        free( tid ) ;
    }
    free( ALIVE_CNT_LOC ) ;
    free( ALIVE_CNT_GOL ) ;
    free( vec_chunk ) ;
    MPI_Barrier( MPI_COMM_WORLD );
    MPI_Finalize();

    return 0;
}


/***************************************************************************/
/* Other Functions - You write as part of the assignment********************/
/***************************************************************************/

// Set parameters by passed arguments argv, ./main thread_per_rank, randomness_threshold, num_ticks 
void SetParameters( char ** arg_vec ) {
    THRD_PER_RANK = atoi( arg_vec[ 1 ] ) ;
    THRESHOLD = atof( arg_vec[ 2 ] ) ;
    TOT_TICKS = atoi( arg_vec[ 3 ] ) ;
    NUM_NODES = MY_MPI_SIZE * THRD_PER_RANK / 64 ;
    sprintf( MY_FNAME , "nds_%d_tpr_%d_thh_%f_tks_%d.bin" , 
             NUM_NODES , THRD_PER_RANK , THRESHOLD , TOT_TICKS ) ;
}

// Use MPI_Reduce to sum counts from all ranks at every tick 
void MyReduce( int * loc_arr , int * glob_arr ) {
    MPI_Barrier( MPI_COMM_WORLD );
    InitializeArray( glob_arr , TOT_TICKS ) ;
    MPI_Reduce( loc_arr , glob_arr , TOT_TICKS , MPI_INT , MPI_SUM, 0 , MPI_COMM_WORLD ) ;
}

//Function to allocation space for the array with size (input_rows*input_cols)
int** Allocation(int input_rows, int input_cols)
{
    int **input_grid = (int **) malloc( input_rows *sizeof(int*));//An array of arry
    int row_index = 0;
    for (; row_index < input_rows; ++row_index) 
    {
        input_grid[row_index] = (int*)malloc( input_cols * sizeof(int) );
    }
    return input_grid;
}

//Function to initialize the array all into ALIVE
void Initialization(int **input_grid_chunk, int input_rows, int input_cols)
{
    int i, j;
    for(i = 0; i < input_rows; i++)
    {
        for(j = 0; j < input_cols; j++)
        { 
            input_grid_chunk[i][j] = (int)ALIVE;
            // input_grid_chunk[i][j] = i + j + MY_MPI_RANK ;
        }
    }
}

// Initialize int array with 0
void InitializeArray( int * my_array , const int arr_size ) {
    int i ;
    for( i = 0 ; i < arr_size ; i++ )
        my_array[ i ] = 0 ;
}

// Function to initialize 1d chunk 2d from grid chunk to facilitate I/O writing
void InitializeLocalChunk( int ** mat_chunk , int * vec_chunk , const int vec_chunk_size ) {
    int i ;
    int row , col ;
    for( i = 0 ; i < vec_chunk_size ; i++ ){
        // +1: excluding ghost row
        row = i / GRID_SIZE + 1 ;
        col = i % GRID_SIZE ;
        if( row > ROWS_PER_RANK ) {
            printf( "Error in func InitializeLocalChunk()\n") ;
            exit( EXIT_FAILURE ) ;
        }
        vec_chunk[ i ] = mat_chunk[ row ][ col ] ;
    }
    if( MY_MPI_RANK == 0 )
        PrintIntArray( vec_chunk , vec_chunk_size , "vec_chunk:" ) ;
}


// Function to send ghost rows
// send last_row to next rank, send first_row to previous rank
void SendGhost( int * first_row , int * last_row ) {
    int rank_dest ;
    MPI_Request sd_reqt_pre , sd_reqt_next ;

    rank_dest = ( MY_MPI_RANK + 1 ) % MY_MPI_SIZE ;
    MPI_Isend(  last_row , GRID_SIZE , MPI_INT , rank_dest , 0 , MPI_COMM_WORLD , & sd_reqt_next ) ;

    if( MY_MPI_RANK == 0 ) { rank_dest = MY_MPI_SIZE - 1 ; }
    else{ rank_dest = MY_MPI_RANK - 1 ; }
    MPI_Isend( first_row , GRID_SIZE , MPI_INT , rank_dest , 0 , MPI_COMM_WORLD , & sd_reqt_pre ) ;
}

// Receive ghost arrays
void RecvGhost( int * recv_pre , int * recv_next ){
    int rank_source ;
    MPI_Status status_pre , status_next ;
    MPI_Request recv_reqt_pre , recv_reqt_next ;

    // ----------------- Receive from previous rank --------------------
    // first rank receives ghost from last rank 
    if( MY_MPI_RANK == 0 ) { rank_source = MY_MPI_SIZE - 1 ; }

    // other ranks receives ghost from previous rank 
    else{ rank_source = MY_MPI_RANK - 1 ; }

    MPI_Irecv( recv_pre , GRID_SIZE , MPI_INT, rank_source , MPI_ANY_TAG , MPI_COMM_WORLD , &recv_reqt_pre );

    // ----------------- Receive from next rank --------------------
    rank_source = ( MY_MPI_RANK + 1 ) % MY_MPI_SIZE ;
    MPI_Irecv( recv_next , GRID_SIZE , MPI_INT, rank_source , MPI_ANY_TAG , MPI_COMM_WORLD , & recv_reqt_next );

    MPI_Wait( &recv_reqt_pre ,&status_pre ) ;
    MPI_Wait( &recv_reqt_next ,&status_next ) ;
}

//Function to free the grid
void FreeGrid(int **input_grid_chunk, int input_rows)
{
    int i;
    for(i = 0; i < input_rows; i++)
    {
        free(input_grid_chunk[i]);
    }
    free(input_grid_chunk);
}

//Function to print the integer array
void PrintIntArray( int* my_arr , const int arr_size , const char * arr_name ) 
{
    int i ;
    printf( "%s\n" , arr_name ) ;
    for ( i = 0 ; i < arr_size ; i++ ) 
        printf( "%d " , my_arr[i] ) ;
    printf( "\n\n" ) ;
}

//Function to print out the grid from start_row to end_row
void PrintGrid( int **input_grid_chunk, int start_row, int end_row)
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

// Count number of alive cells in grid
int CountAliveCellsOnGrid( int ** grid_chunk , int start_row , int end_row )
{
    int i, j;
    int total_alive_cells_number = 0;

    for(i = start_row; i < end_row+1; i++)
    {
        for(j = 0; j < GRID_SIZE; j++)
        {
            if (grid_chunk[i][j] == (int)ALIVE)
            {
                total_alive_cells_number+=1;
            }
        }
    }

    return total_alive_cells_number;
}

// Count number of alive cells among 8 neighbors of a cell
int CountAdjacentAliveCells(int **input_grid_chunk, const int cell_row_index, const int cell_col_index)
{
    int number_alive_neighbors = 0;
    //There are 8 neighbors for each given cell_col_index
    int i, j;
	//Check if the given row index out of range (we should not count for the ghost rows)
    if ( cell_row_index < 1 || cell_row_index > GRID_SIZE / MY_MPI_SIZE )
    {
        printf("Row Index Out Of Range  %d ! \n", cell_row_index);
        exit( EXIT_FAILURE ) ;
    }
	//need three rows index for neighbors
    for(i = cell_row_index-1; i < cell_row_index+2; i++)
    {
        if (cell_col_index == 0)//the leftmost column cells need info from the last column
        {
            if (input_grid_chunk[i][GRID_SIZE-1] == (int)ALIVE)
            {
                number_alive_neighbors+=1;
            }
            if (input_grid_chunk[i][cell_col_index] == (int)ALIVE)
            {
                number_alive_neighbors+=1;
            }
            if (input_grid_chunk[i][cell_col_index+1] == (int)ALIVE)
            {
                number_alive_neighbors+=1;
            }
        }
        else if (cell_col_index == GRID_SIZE-1)//the rightmost column cells need info from the first column
        {
            if (input_grid_chunk[i][cell_col_index-1] == (int)ALIVE)
            {
                number_alive_neighbors+=1;
            }
            if (input_grid_chunk[i][cell_col_index] == (int)ALIVE)
            {
                number_alive_neighbors+=1;
            }
            if (input_grid_chunk[i][0] == (int)ALIVE)
            {
                number_alive_neighbors+=1;
            }
        }
        else//middle column cells
        {
            for(j = cell_col_index-1; j < cell_col_index+2; j++)
            {
                if (input_grid_chunk[i][j] == (int)ALIVE)
                {
                    number_alive_neighbors+=1;
                }
            }
        }
    }

    //exclude the state of the given cell
    if (input_grid_chunk[cell_row_index][cell_col_index] == (int)ALIVE)
    {
        number_alive_neighbors-=1;
    }

    return number_alive_neighbors;

}

//Function to update the cell status based on the given rules
void UpdateCellWithBasicRules(int **input_grid_chunk, int start_row, int end_row)
{
    int i, j;
    for(i = start_row; i < end_row+1; i++)//update the rows from start_row to end_row
    {
        for(j = 0; j < GRID_SIZE; j++)
        {
            int my_alive_neighbor_num = CountAdjacentAliveCells(input_grid_chunk, i,j);
            // ALIVE, < 2 live neighbor, changed to be DEAD
            if ( input_grid_chunk[i][j] == (int)ALIVE && my_alive_neighbor_num < 2 )
            {
                input_grid_chunk[i][j] = (int)DEAD;
            }

            // ALIVE, 2 or 3 live neighbors, still ALIVE, no need to change
            
            // ALIVE, > 3 live neighbors, changed to be DEAD
            else if ( input_grid_chunk[i][j] == (int)ALIVE && my_alive_neighbor_num > 3)
            {
                input_grid_chunk[i][j] = (int)DEAD;
            }

            // DEAD, = 3 live neighbors, changed to be ALIVE 
            else if ( input_grid_chunk[i][j] == (int)DEAD && my_alive_neighbor_num == 3)
            {
                input_grid_chunk[i][j] = (int)ALIVE;
            }

			//In other cases, status of cell remains the same 
        }
    }
}

//Function to update the cell status randomly
void UpdateCellRandomly(int **input_grid_chunk, int start_row, int end_row )
{
    int i, j;
    int global_row ;
    for( i = start_row ; i < end_row + 1 ; i++ )
    {
        //global row index for RNG
        global_row = start_row - 1 + ( ROWS_PER_RANK * MY_MPI_RANK ) ;
        for(j = 0; j < GRID_SIZE; j++)
        {
            //50 percent possibility to be ALIVE
            if ( GenVal( global_row ) < 0.5 ) 
            {
                input_grid_chunk[i][j] = (int)ALIVE;
            }
            else
            {
                input_grid_chunk[i][j] = (int)DEAD;
            }
        }
    }
}


/* 
 * Play game of life 
 *
 * id = 0, MPI master thread 
 *      (1) Exchange ghost rows by func SendGhost() and RecvGhost() 
 *      (2) Play game 
 *      (3) Update ALIVE_CNT_LOC at the end of each tick  
 *
 * id = 1..THD_NUM, threads created 
 *      Just play game
 */
void *PlayGame( void * arg )
{
    //int * id =  ( int * ) arg ;
    int id = * ( ( int * ) arg ) ;

    int rows_per_thrd = ROWS_PER_RANK / THRD_PER_RANK ; 
    int start_row = id * rows_per_thrd + 1 ;
    int end_row = start_row + rows_per_thrd ;

    int global_row , i_tk , i_row ;
    for( i_tk = 0 ; i_tk < TOT_TICKS ; i_tk++ ) {

        // Master thread execute Isend/Irecv
        if( id == 0 ) {
            SendGhost( LOCAL_UNIVERSE[ 1 ] , LOCAL_UNIVERSE[ ROWS_PER_RANK ] ) ;
            RecvGhost( LOCAL_UNIVERSE[ 0 ] , LOCAL_UNIVERSE[ ROWS_PER_RANK + 1 ]  ) ;
        }

        // Block until all threads arrive here 
        //pthread_barrier_wait( & THRD_BARRIER ) ;

        // All threads play game
        for( i_row = start_row ; i_row < end_row ; i_row++ ) {
            global_row = i_row - 1 + ( ROWS_PER_RANK * MY_MPI_RANK ) ;
            if( GenVal( global_row ) < THRESHOLD ) {
                UpdateCellRandomly( LOCAL_UNIVERSE , i_row , i_row ) ;
            }
            else {
                UpdateCellWithBasicRules( LOCAL_UNIVERSE , i_row , i_row ) ; 
            }
        } 

        // Block until all threads arrive 
        pthread_barrier_wait( & THRD_BARRIER ) ;

        // Master thread counts num of alive cells in LOCAL_UNIVERSE
        if( id == 0 ) {
            if( MY_MPI_RANK == 0 ) {
                //PrintGrid( LOCAL_UNIVERSE , 0 , ROWS_PER_RANK + 1 ) ;
                //printf( "\n\n" ) ;
            }
            ALIVE_CNT_LOC[ i_tk ] = CountAliveCellsOnGrid( LOCAL_UNIVERSE , 1 , ROWS_PER_RANK ) ;
        }
    }

    return NULL ;
}

// Print key parameters
void PrintInitConditions() {
        printf( "Initial Conditions:\n" ) ;
        printf( "\tMY_MPI_SIZE = %d\n" , MY_MPI_SIZE ) ;
        printf( "\tTHRD_PER_RANK = %d\n" , THRD_PER_RANK ) ;
        printf( "\tTHRESHOLD = %f\n" , THRESHOLD ) ;
        printf( "\tTOT_TICKS = %d\n\n" , TOT_TICKS ) ;
}
