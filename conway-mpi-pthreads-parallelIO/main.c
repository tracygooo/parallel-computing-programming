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
//#define GRID_SIZE 32768
#define GRID_SIZE 32 

/***************************************************************************/
/* Global Vars *************************************************************/
/***************************************************************************/

double g_time_in_secs = 0;
double g_processor_frequency = 1600000000.0; // processing speed for BG/Q
unsigned long long g_start_cycles=0;
unsigned long long g_end_cycles=0;

// You define these
int number_of_ticks = 2 ;
int given_threshold = 0.0; 
int mpi_commsize;
int BGQ_nodes_num = 4;
int rank_per_node =0;
pthread_barrier_t mybarrier;

//pass multiple arguments to the thread
struct thread_args {
    int** local_grid;
    int thread_id;
    int rows_per_thread;
    double update_threshold;
    int mpi_rank_index; 
    int rows_per_rank;
    unsigned long long *tick_sum_array;
};




/***************************************************************************/
/* Function Decs ***********************************************************/
/***************************************************************************/

// You define these
int** Allocation(int input_rows, int input_cols);
void Initialization(int **input_grid_chunk, int input_rows, int input_cols);
void PrintGrid(int **input_grid_chunk, int start_row, int end_row);
void FreeGrid(int **input_grid_chunk, int input_rows);
int CountAdjacentAliveCells(int **input_grid_chunk, int cell_row_index, int cell_col_index);
void UpdatCellWithBasicRules(int **input_grid_chunk, int start_row, int end_row);
void UpdatCellRandomly(int **input_grid_chunk, int start_row, int end_row);
unsigned long long CountAliveCellsOnGrid(int **input_grid_chunk, int start_row, int end_row);
void *PlayGameOfLife(void* input_thread_args);
void CopyRow(int **input_grid_chunk, int *new_array, int loc_index, int direction);

//Function to print the integer array
void print_int_array( const int* my_arr , const int my_size , const char * arr_name ) ;
void print_ull_array( const unsigned long long * my_arr , const int my_size , const char * arr_name );


/***************************************************************************/
/* Function: Main **********************************************************/
/***************************************************************************/

int main(int argc, char *argv[])
{
    int mpi_myrank;

    //-------- Parameters for MPI I/O -----------------
    int * r_buff ; // buffer for MPI_File_read_at
    MPI_File fh;
    MPI_Offset offset ;
    MPI_Status status;
    int chunk_int_num ; // Number of chunk array in each MPI rank

    unsigned long long *ALIVE_count_global = (unsigned long long *) malloc( sizeof(unsigned long long) * number_of_ticks);
    int global_sum_index;

    for (global_sum_index=0; global_sum_index<number_of_ticks; global_sum_index++)
    {
        ALIVE_count_global[global_sum_index] = 0;
    }

    // Example MPI startup and using CLCG4 RNG
    MPI_Init( &argc, &argv);
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_commsize);
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_myrank);


    // ===============================================================
    // Person A -----------Start1 ---------------
    //      RNG streams
    // ===============================================================
    // Init 32,768 RNG streams - each rank has an independent stream
    InitDefault();

    // Note, used the mpi_myrank to select which RNG stream to use.
    // You must replace mpi_myrank with the right row being used.
    // This just show you how to call the RNG.    
    printf("Rank %d of %d has been started and a first Random Value of %lf\n", 
            mpi_myrank, mpi_commsize, GenVal(mpi_myrank));
    // ===============================================================
    // Person A -----------End1---------------
    // ===============================================================

    MPI_Barrier( MPI_COMM_WORLD );


    // ===============================================================
    // Person B -----------Start ---------------
    //      Pthreads 
    //      Count alive cells 
    // ===============================================================
    // Insert your code
    /************************************ My Codes Start Here ****************************************/
    //if rank 0 / thread 0, start time with GetTimeBase().
    if (0 == mpi_myrank)
    {
        g_start_cycles = GetTimeBase();

    }

    //Each rank need to have a vector of 256 ALIVE cell sum
    unsigned long long *ALIVE_count_per_rank = (unsigned long long *) malloc( sizeof(unsigned long long) * number_of_ticks);
    int init_sum_index;

    for (init_sum_index=0; init_sum_index<number_of_ticks; init_sum_index++)
    {
        ALIVE_count_per_rank[init_sum_index] = 0;
    }
    //Allocate My ranks chunk of the universe + space for "ghost" rows.
    //int thread_to_mpi_ratio =  BGQ_nodes_num*rank_per_node/mpi_commsize;
    int thread_to_mpi_ratio =  4;
    int num_threads = thread_to_mpi_ratio-1;

    //unsigned long long global_alive_sum =0;
    int universe_per_rank = (int)GRID_SIZE/mpi_commsize; // number of rows per rank
    int universe_and_ghost_rows = universe_per_rank+2;
    int num_rows_per_thread = universe_per_rank/thread_to_mpi_ratio;
    int **local_universe = Allocation (universe_and_ghost_rows,GRID_SIZE);

    int grid_start_row = 1;
    int grid_end_row = universe_per_rank;

    //Initalize the universe (including "ghost" rows) with every cell being ALIVE.
    Initialization(local_universe,universe_and_ghost_rows,GRID_SIZE);

    //Create Pthreads here. All threads should go into the for-loop.
    pthread_t p_threads[num_threads]; 
    pthread_attr_t attr; 
    pthread_attr_init (&attr); 

    pthread_barrier_init(&mybarrier, NULL, num_threads);
    //printf("number of threads %d \n", num_threads);
    int j;
    for (j=0; j< num_threads; j++) 
    {
        struct thread_args *args_per_thread = (struct thread_args *)malloc(sizeof(struct thread_args));
        args_per_thread-> local_grid = local_universe;
        args_per_thread-> thread_id = j ;
        args_per_thread-> rows_per_thread = num_rows_per_thread;
        args_per_thread-> update_threshold = given_threshold;
        args_per_thread-> mpi_rank_index = mpi_myrank;
        args_per_thread-> rows_per_rank = universe_per_rank;
        args_per_thread-> tick_sum_array = ALIVE_count_per_rank;
        pthread_create(&p_threads[j], &attr, PlayGameOfLife,(void *) args_per_thread); 
    }

    //MPI process this portion
    //printf(" this is rank %d in the main \n", mpi_myrank);
    struct thread_args *args_mpi = (struct thread_args *)malloc(sizeof(struct thread_args));
    args_mpi-> local_grid = local_universe;
    args_mpi-> thread_id = GRID_SIZE ;
    args_mpi-> rows_per_thread = num_rows_per_thread;
    args_mpi-> update_threshold = given_threshold;
    args_mpi-> mpi_rank_index = mpi_myrank;
    args_mpi-> rows_per_rank = universe_per_rank;
    args_mpi-> tick_sum_array = ALIVE_count_per_rank;
    PlayGameOfLife(args_mpi);

    MPI_Reduce(ALIVE_count_per_rank, ALIVE_count_global,number_of_ticks, MPI_LONG_LONG, MPI_SUM, 0,MPI_COMM_WORLD);

    if (0 == mpi_myrank)
    {
        //print_ull_array(  ALIVE_count_global, number_of_ticks, "global count for ALIVE cells" ) ;
        free(ALIVE_count_global);
    }

    free(ALIVE_count_per_rank);

    //if rank 0 / thread 0, end time with GetTimeBase().
    // ===============================================================
    // Person A -----------End2---------------
    // ===============================================================

    //pthread_barrier_wait(&mybarrier);
    //Clean the threads
    for (j=0; j< num_threads; j++)
    {
        pthread_join(p_threads[j], NULL);

    }

    //printf("I am after the pthread koin process \n");
    pthread_barrier_destroy(&mybarrier);
    
    // ===============================================================
    // Person B -----------End ---------------
    // ===============================================================



    // ===============================================================
    // Person A -----------Start2 ---------------
    //      Conduct MPI I/O
    //      Construct heatmap via gnuplot or other tools 
    // ===============================================================

    //if needed by experiment,
    //-perform output of 32Kx32K cell universe using MPI_file_write_at;
    //-collect I/O performance stats using GetTimeBase() from rank 0 / thread 0;

    // ----------------------------------------------------------------------------------------------------
    // Write chunk array from buffer to file "universe.bin"
    // ----------------------------------------------------------------------------------------------------

    chunk_int_num = GRID_SIZE * GRID_SIZE / mpi_commsize ;
    offset = mpi_myrank * chunk_int_num * sizeof( int ) ;

    int row_per_rank = GRID_SIZE / mpi_commsize ;
    MPI_File_open( MPI_COMM_WORLD, "universe.bin", MPI_MODE_RDWR | MPI_MODE_CREATE , MPI_INFO_NULL, &fh ) ;
    int i ;
    for( i = 0 ; i < row_per_rank ; i++ ) {
        MPI_File_write_at( fh , offset , local_universe[ i + 1 ] , GRID_SIZE , MPI_INT, &status );
        //MPI_File_write_at( fh , offset , local_universe1[ i ] , GRID_SIZE , MPI_INT, &status );
        offset += GRID_SIZE * sizeof( int ) ;
    }
    MPI_Barrier( MPI_COMM_WORLD );

    // ----------------------------------------------------------------------------------------------------
    // Read chunk array from file to buffer 
    // ----------------------------------------------------------------------------------------------------
    r_buff = ( int * ) malloc( chunk_int_num * sizeof( int ) ) ;
    offset = mpi_myrank * chunk_int_num * sizeof( int ) ;
    MPI_File_read_at( fh , offset , r_buff, chunk_int_num , MPI_INT , &status ) ;
    MPI_Barrier( MPI_COMM_WORLD );
    for( j = 0 ; j < mpi_commsize ; j++ ) {
        if( mpi_myrank == j ) {
            for( i = 0 ; i < chunk_int_num ; i++ ) {
                printf( "%d " , r_buff[ i ] ) ;
            }
        }
    }

    /*
    */
    MPI_File_close( &fh ); 
    //if needed by experiment,
    //construct 1Kx1K heatmap of 32Kx32K cell universe using MPI collective operations of your choice. Rank 0 will output
    //the heatmap to a standard Unix file given its small 1 to 4MB size.
    //Make sure you an import data for graphing.

    //if rank 0, print ALIVE tick stats and compute (I/O if needed) performance stats.



    /************************************ My Codes End Here ****************************************/

    // END -Perform a barrier and then leave MPI
    MPI_Barrier( MPI_COMM_WORLD );
    FreeGrid( local_universe, universe_and_ghost_rows );
    //FreeGrid( local_universe1 , GRID_SIZE );
    MPI_Finalize();
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

void FreeGrid(int **input_grid_chunk, int input_rows)
{
    int i;
    for(i = 0; i < input_rows; i++)
    {
        free(input_grid_chunk[i]);
    }
    free(input_grid_chunk);
}

int CountAdjacentAliveCells(int **input_grid_chunk, int cell_row_index, int cell_col_index)
{
    int number_alive_neighbors = 0;
    //There are 8 neighbors for each given cell_col_index
    int i, j;

    if (cell_row_index<1 || cell_row_index > GRID_SIZE)
    {
        printf("Row Index Out Of Range  %d ! \n", cell_row_index);
    }
    for(i = cell_row_index-1; i < cell_row_index+2; i++)
    {
        if (cell_col_index == 0)
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
        else if (cell_col_index == GRID_SIZE-1)
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
        else
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

void UpdatCellWithBasicRules(int **input_grid_chunk, int start_row, int end_row)
{
    int i, j;
    for(i = start_row; i < end_row+1; i++)
    {
        for(j = 0; j < GRID_SIZE; j++)
        {
            int my_alive_neighbor_num = CountAdjacentAliveCells(input_grid_chunk, i,j);
            if (my_alive_neighbor_num < 2)
            {
                input_grid_chunk[i][j] = (int)DEAD;
            }
            else if (my_alive_neighbor_num > 3)
            {
                input_grid_chunk[i][j] = (int)DEAD;
            }
            else
            {
                input_grid_chunk[i][j] = (int)ALIVE;
            }
        }
    }

}

void UpdatCellRandomly(int **input_grid_chunk, int start_row, int end_row)
{
    int i, j;
    for(i = start_row; i < end_row+1; i++)
    {
        for(j = 0; j < GRID_SIZE; j++)
        {
            double returned_random_value = drand48();
            if (returned_random_value<0.5)
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

unsigned long long CountAliveCellsOnGrid(int **input_grid_chunk, int start_row, int end_row)
{
    int i, j;
    unsigned long long total_alive_cells_number = 0;

    for(i = start_row; i < end_row+1; i++)
    {
        for(j = 0; j < GRID_SIZE; j++)
        {
            if (input_grid_chunk[i][j] == (int)ALIVE)
            {
                total_alive_cells_number+=1;
            }
        }
    }

    return total_alive_cells_number;
}


void *PlayGameOfLife(void* input_thread_args)
{
    int** my_grid = ((struct thread_args*)input_thread_args)->local_grid;
    int my_id = ((struct thread_args*)input_thread_args)->thread_id;
    int my_rows_num = ((struct thread_args*)input_thread_args)->rows_per_thread; 
    double my_threshold = ((struct thread_args*)input_thread_args)-> update_threshold; 
    int my_mpi_rank = ((struct thread_args*)input_thread_args)-> mpi_rank_index; 
    int my_rows_per_rank = ((struct thread_args*)input_thread_args)->rows_per_rank;
    unsigned long long *my_tick_sum = ((struct thread_args*)input_thread_args)->tick_sum_array;


    int row_start_index;
    if (my_id <GRID_SIZE)
    {
        row_start_index = (my_id+1) * my_rows_num+1;
    }
    else
    {
        row_start_index = 1;
    }
    int row_end_index = row_start_index + my_rows_num-1;

    int i,j;
    for( i = 0; i < number_of_ticks; i++)
    {
        pthread_mutex_t count_mutex;
        if (my_id == GRID_SIZE)
        {
            //Exchange row data with MPI ranks using MPI_Isend/Irecv from thread 0 w/i each MPI rank. 
            //decide the last and next rank index
            int next_rank, last_rank;
            if ( my_mpi_rank == 0)
            {
                next_rank = 1;
                last_rank = mpi_commsize-1;
            }
            else if (my_mpi_rank == mpi_commsize-1)
            {
                next_rank = 0;
                last_rank = mpi_commsize-2;
            }
            else
            {
                next_rank = my_mpi_rank+1;
                last_rank = my_mpi_rank-1;
            }



            MPI_Status   status1,status2,status3,status4 ;
            MPI_Request send_request_next,recv_request_next,send_request_above,recv_request_above;
            int *send_to_next, *send_to_above;
            int *recv_from_next, *recv_from_above;


            send_to_next = (int *) malloc( sizeof(int) * GRID_SIZE);
            send_to_above = (int *) malloc( sizeof(int) * GRID_SIZE);
            recv_from_next = (int *) malloc( sizeof(int) * GRID_SIZE);
            recv_from_above = (int *) malloc( sizeof(int) * GRID_SIZE);

            int index_mpi_sendrecv;
            for (index_mpi_sendrecv = 0; index_mpi_sendrecv < GRID_SIZE; index_mpi_sendrecv++)
            {
                send_to_next[index_mpi_sendrecv] = 0;
                send_to_above[index_mpi_sendrecv] = 0;
                recv_from_next[index_mpi_sendrecv] = 0;
                recv_from_above[index_mpi_sendrecv] = 0;
            }

            //send to next rank
            CopyRow(my_grid, send_to_next,my_rows_per_rank,1);
            MPI_Isend(send_to_next,GRID_SIZE,MPI_INT,next_rank,0,MPI_COMM_WORLD,&send_request_next);
            MPI_Wait(&send_request_next,&status1);

            //send to previous rank
            CopyRow(my_grid, send_to_above,1, 1);
            MPI_Isend(send_to_above,GRID_SIZE,MPI_INT,last_rank,0,MPI_COMM_WORLD,&send_request_above);
            MPI_Wait(&send_request_above,&status2);

            //recv from last rank
            MPI_Irecv(recv_from_above,GRID_SIZE,MPI_INT,last_rank,MPI_ANY_TAG,MPI_COMM_WORLD,&recv_request_above);

            //recv from next rank
            MPI_Irecv(recv_from_next,GRID_SIZE,MPI_INT,next_rank,MPI_ANY_TAG,MPI_COMM_WORLD,&recv_request_next);


            MPI_Wait(&recv_request_above,&status3);
            CopyRow(my_grid, recv_from_above,0,0);

            MPI_Wait(&recv_request_next,&status4);
            CopyRow(my_grid, recv_from_next,my_rows_per_rank+1, 0);


            free(recv_from_above);
            free(recv_from_next);
            free(send_to_above);
            free(send_to_next);
        }


        if (my_id < GRID_SIZE)
        {
            pthread_barrier_wait(&mybarrier);
            //printf("I am after the barrier : mpi %d  thread %d  tick %d \n", my_mpi_rank, my_id, i);
        }

        for (j = row_start_index; j < row_end_index; j++)
        {
            //HERE each PTHREAD can process a row:
            // - update universe making sure to use the correct row RNG stream
            int RNG_index = j + (my_rows_per_rank * my_mpi_rank);
            //double my_random_number = ...
            double my_random_number = 0.1234;
            // - factor in Threshold percentage as described
            // - use the right "ghost" row data at rank boundaries
            unsigned long long  my_sum_per_tick = 0;
            // - keep track of total number of ALIVE cells per tick across all threads w/i a MPI rank group.
            // - use pthread_mutex_trylock around shared counter variables.
            if (my_random_number < my_threshold)
            {
                UpdatCellRandomly(my_grid,row_start_index,row_end_index);
                my_sum_per_tick = CountAliveCellsOnGrid( my_grid,row_start_index, row_end_index);
                my_tick_sum[i]+=my_sum_per_tick;
                //printf("I am in the randomly update section ! \n");
            }
            else
            {
                UpdatCellWithBasicRules(my_grid,row_start_index,row_end_index);
                my_sum_per_tick = CountAliveCellsOnGrid( my_grid,row_start_index,row_end_index);
                pthread_mutex_trylock(&count_mutex); 
                my_tick_sum[i]+=my_sum_per_tick;
                pthread_mutex_unlock(&count_mutex);
            }

        }

    }
    free(input_thread_args);
    return NULL;
}

void CopyRow(int **input_grid_chunk, int *new_array, int loc_index, int direction)
{
    int i;
    //put in the new ghost row content from recv
    if (direction == 0)
    {
        for (i=0; i<GRID_SIZE; i++)
        {
            input_grid_chunk[loc_index][i] = new_array[i];
        }
    }
    else // get the content from the updated local grid to send
    {
        for (i=0; i<GRID_SIZE; i++)
        {
            new_array[i] = input_grid_chunk[loc_index][i];
        }
    }
}

void print_int_array( const int* my_arr , const int my_size , const char * arr_name ) 
{
    int i ;
    printf( "%s\n" , arr_name ) ;
    for ( i = 0 ; i < my_size ; i++ ) 
        printf( "%d" , my_arr[i] ) ;
    printf( "\n\n" ) ;
}
void print_ull_array( const unsigned long long * my_arr , const int my_size , const char * arr_name )
{
    int i ;
    printf( "%s\n" , arr_name ) ;
    for ( i = 0 ; i < my_size ; i++ )
        printf( "%llu  " , my_arr[i] ) ;
    printf( "\n\n" ) ;
}

