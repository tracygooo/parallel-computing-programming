#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<math.h>
#include<pthread.h>
#include<mpi.h>

#define BGQ 1 // when running BG/Q, comment out when testing on mastiff

#ifdef BGQ
#include<hwi/include/bqc/A2_inlines.h>
#else
#define GetTimeBase MPI_Wtime            
#endif

#define NUM_ROWS_C 8192 
#define NUM_COLUMNS_C NUM_ROWS_C 
#define NUM_MIDDLE_AB NUM_ROWS_C 

//Execution time calculation
double g_time_in_secs = 0;
double g_processor_frequency = 1600000000.0; // processing speed for BG/Q
//double g_processor_frequency = 1 ; // processing speed for mastiff
unsigned long long g_start_cycles=0;
unsigned long long g_end_cycles=0;

double g_time_in_secs_overhead = 0;
unsigned long long g_start_cycles_overhead=0;
unsigned long long g_end_cycles_overhead=0;

int UPPER_LIMIT = 100;//For random initialization
int NUM_NODES;
int THRD_PER_RANK;// number of threads per rank
int THRD_CREATED; // THRD_PER_RANK - 1
int MY_MPI_RANK ;
int MY_MPI_SIZE ;
int ROWS_PER_RANK;// number of rows processed by each rank
int * A_mtx_local;
int * B_mtx;
int * C_mtx;
int * C_mtx_local;

//Function Definitions
int * InitializeMatrix(const char initialization_option, const char matrix_option,int rows_per_rank, int upper_limit);
void UnsquareMultiplication(int start_index_A, int end_index_A, int num_columns_B,int *matrix_A,int *matrix_B, int *matrix_C);
void *StripMatrix( void * arg );
void FreeMatrix(int *input_matrix);
void PrintMatrix(int *input_matrix, const int num_rows, const int num_cols);
void PrintInitConditions() ;

// ./main thread_per_rank 
void SetParameters( char ** arg_vec ) {
    THRD_PER_RANK = atoi( arg_vec[ 1 ] ) ;
    NUM_NODES = MY_MPI_SIZE * THRD_PER_RANK / 64 ;
}




int main(int argc, char *argv[])
{
	// Init MPI
    MPI_Init( &argc, &argv);
    MPI_Comm_size( MPI_COMM_WORLD, & MY_MPI_SIZE );
    MPI_Comm_rank( MPI_COMM_WORLD, & MY_MPI_RANK );
	// Init parameters, ./main thread_per_rank
    SetParameters( argv ) ;	
    if( MY_MPI_RANK == 0 ) {
	    PrintInitConditions() ;
    }
	
	// Init global variables 
	ROWS_PER_RANK = NUM_ROWS_C / MY_MPI_SIZE ;//Decide the number of rows per rank
	//Matrix initialization
	A_mtx_local = InitializeMatrix('O', 'A', ROWS_PER_RANK,UPPER_LIMIT);
	B_mtx = InitializeMatrix('O', 'B',NUM_ROWS_C,UPPER_LIMIT );
	C_mtx_local = InitializeMatrix('Z', 'C',ROWS_PER_RANK,UPPER_LIMIT );


	// if rank 0 / thread 0, start time with GetTimeBase()for execution time.
    if (0 == MY_MPI_RANK )
    {

        //PrintMatrix(A_mtx_local, ROWS_PER_RANK, NUM_MIDDLE_AB);
		//PrintMatrix(B_mtx, NUM_MIDDLE_AB, NUM_COLUMNS_C );
        g_start_cycles = GetTimeBase();
    }

	
	
	
	// -----------------------------------------------------------------
    // Create Threads and threads do their section of matrix multiplication
    // -----------------------------------------------------------------
	THRD_CREATED =  THRD_PER_RANK-1;//The actual number of threads needed
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
            if( pthread_create( &p_threads[ i - 1 ], &attr, StripMatrix , & tid[ i - 1 ]  ) != 0 ){  
                fprintf( stderr, "MAIN: Could not create thread (%d)\n", i );
                return EXIT_FAILURE;
            }
        }
    }
	
	// -----------------------------------------------------------------
    // MPI master thread do its section of matrix multiplication
    // -----------------------------------------------------------------
    int master_id = 0 ;
    StripMatrix( & master_id ) ; 

    //Clean the threads
    if( THRD_CREATED != 0 ) {
        for ( i = 0 ; i < THRD_CREATED ; i++)
        {
            pthread_join( p_threads[ i ] , NULL);
        }
    }
	
	//if rank 0 / thread 0, make a global result matrix
    if (0 == MY_MPI_RANK )
    {
		C_mtx = InitializeMatrix('Z', 'C', NUM_ROWS_C,UPPER_LIMIT);
    }
	
	// if rank 0 / thread 0, start time with GetTimeBase()for overhead.
    if (0 == MY_MPI_RANK )
    {
        g_start_cycles_overhead = GetTimeBase();
    }
    MPI_Barrier( MPI_COMM_WORLD );
	
	//Collect results from all MPI ranks to the global result matrix
	MPI_Gather( C_mtx_local,ROWS_PER_RANK*NUM_ROWS_C, MPI_INT, C_mtx, ROWS_PER_RANK*NUM_ROWS_C, MPI_INT, 0, MPI_COMM_WORLD); 

    MPI_Barrier( MPI_COMM_WORLD );
	//if rank 0 / thread 0, end time with GetTimeBase() for overhead and execution time
    if (0 == MY_MPI_RANK )
    {
		g_end_cycles_overhead = GetTimeBase() ;
        g_end_cycles = GetTimeBase() ;
		g_time_in_secs_overhead = ((double)( g_end_cycles_overhead - g_start_cycles_overhead ) ) / g_processor_frequency;
        g_time_in_secs = ((double)( g_end_cycles - g_start_cycles ) ) / g_processor_frequency;
        printf("Overhead time =  %f \n", g_time_in_secs_overhead);
		printf("Total execution time =  %f \n", g_time_in_secs);
		printf("Real Compute execution time =  %f \n", g_time_in_secs - g_time_in_secs_overhead);
		//PrintMatrix(C_mtx, NUM_ROWS_C, NUM_COLUMNS_C);
		FreeMatrix(C_mtx);
    }

    FreeMatrix(A_mtx_local);
    FreeMatrix(B_mtx);
    FreeMatrix(C_mtx_local);
    return 0;
}

//Function to initialize the matrix
int *InitializeMatrix(const char initialization_option, const char matrix_option, int rows_per_rank, int upper_limit)
{
	
	int total_cell_num;
	
	//Matrices with different sizes are needed to be processed differently
	if (matrix_option == 'A')
	{
		total_cell_num = rows_per_rank * NUM_MIDDLE_AB;
	}
	else if (matrix_option == 'B')
	{
		total_cell_num = rows_per_rank * NUM_COLUMNS_C;
	}
	else if (matrix_option == 'C')
	{
		total_cell_num = rows_per_rank * NUM_COLUMNS_C;
	}
	
	//Allocate a 1D array for the matrix
	int *my_matrix = ( int * ) malloc( sizeof( int ) * total_cell_num ) ;
	
	int cell_i;
	
	//Initialization Option 1 (Global Index): Dynamically allocate the matrix and initialize based on the global index of each cell
	if (initialization_option == 'G')
	{
	        if (matrix_option == 'B')//B must be initialized as a complete n*n matrix
        	{	
            		cell_i = 0;
            		for ( ; cell_i < total_cell_num ; cell_i++)
            		{
                		my_matrix[cell_i] = cell_i;
           		 }
        	}
		//Matrix A and C are local with assigned rows for each rank. So to initialize them globally, we need to count the number of cells from other mpi ranks with index smaller than the current rank.
        	else
        	{	
            		cell_i = MY_MPI_RANK*total_cell_num ;


            		for ( ; cell_i < ( MY_MPI_RANK + 1 ) * total_cell_num ; cell_i++)
            		{
                		my_matrix[cell_i - MY_MPI_RANK * total_cell_num ] = cell_i;
            		}
        	}	

	}
	//Initialization Option 2 (All Ones): Dynamically allocate the matrix and initialize to ones 
	else if (initialization_option == 'O')
	{
		for (cell_i = 0 ; cell_i < total_cell_num ; cell_i++)
		{
			my_matrix[cell_i] = 1;
		}
	}
	//Initialization Option 3 (All Zeros): Dynamically allocate the matrix and initialize to zeros 
	else if (initialization_option == 'Z')
	{
		for (cell_i = 0 ; cell_i < total_cell_num ; cell_i++)
		{
			my_matrix[cell_i] = 0;
		}
	}
	//Initialization Option 4 (Random Numbers): Dynamically allocate the matrix and initialize to random numbers 
	else if (initialization_option == 'R')
	{
		for (cell_i = 0 ; cell_i < total_cell_num ; cell_i++)
		{
			my_matrix[cell_i] = rand()%upper_limit; //upper_limit can constrain the range of generated random numbers
		}
	}
	else
	{
		printf("ERROR: The initialization option can only be G (global index) or O (all ones) or Z (all zeros) or R(random numbers)! \n");
	}
	
	return my_matrix;
}

//Function to do the given section of matrix multiplication
void UnsquareMultiplication(int start_index_A, int end_index_A, int num_columns_B,int *matrix_A,int *matrix_B,int *matrix_C )
{
	int a_index, b_index, c_index;
	int local_sum;
	for(a_index=start_index_A;a_index<end_index_A;a_index++)
	{
		for(b_index=0;b_index<num_columns_B;b_index++)
		{
			local_sum = 0;
			for(c_index=0;c_index<num_columns_B;c_index++)
			{
				//calculate C = A*B
				local_sum += matrix_A[a_index * num_columns_B + c_index] * matrix_B[c_index * num_columns_B + b_index];  
			}
			matrix_C[a_index * num_columns_B+ b_index] = local_sum;	
		}
	}
}

//Function for the thread to perform the task
void *StripMatrix( void * arg )
{
  int id = * ( ( int * ) arg ) ;
  //For matrix A: get the rows that this thread need to process
  int rows_per_thrd = ROWS_PER_RANK / THRD_PER_RANK; 
  int start_row = id * rows_per_thrd;
  int end_row = start_row + rows_per_thrd ;

  //Matrix Multiplication for unsquare Matrix
  UnsquareMultiplication(start_row, end_row, NUM_COLUMNS_C, A_mtx_local, B_mtx, C_mtx_local );
  return NULL ;
}

//Free the matrix
void FreeMatrix(int *input_matrix)
{
	free(input_matrix);
}

//Print the matrix cells
void PrintMatrix(int *input_matrix,const int num_rows, const int num_cols)
{
	int row_i_print, col_j_print;
	int global_index  = 0;
	for(row_i_print=0;row_i_print<num_rows;row_i_print++)
	{
		for(col_j_print=0;col_j_print<num_cols;col_j_print++)
		{
			printf("%d\t",input_matrix[global_index]);
			global_index++;
		}    
		printf("\n");
	}
	printf("\n");
	printf("\n");
}

void PrintInitConditions() {
        printf( "Initial Conditions:\n" ) ;
        printf( "\tNUM_NODES = %d\n" , NUM_NODES ) ;
        printf( "\tMY_MPI_SIZE = %d\n" , MY_MPI_SIZE ) ;
        printf( "\tTHRD_PER_RANK = %d\n" , THRD_PER_RANK ) ;

}


