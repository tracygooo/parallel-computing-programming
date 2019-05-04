
/***************************************************************************/
/*
 * Implemented cannon's algorithm for matrix multiplication C = A * B 
 * Write matrix C to a binary file  
 * Jinghua Feng, fengj3@rpi.edu 
 * Miao Qi, qim@rpi.edu 
 */

/***************************************************************************/
/* Includes ****************************************************************/
/***************************************************************************/
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<math.h>
#include<mpi.h>

//#define BGQ 1 // when running BG/Q, comment out when testing on mastiff
#ifdef BGQ
    #include<hwi/include/bqc/A2_inlines.h>
    double PROC_FREQ = 1600000000.0 ; // processor frequency
#else
    double PROC_FREQ = 1.0;
    #define GetTimeBase MPI_Wtime            
#endif


/***************************************************************************/
/* Macros ******************************************************************/
/***************************************************************************/
#define TRUE 1
#define FALSE 0 

#define MY_MPI_DT MPI_INT // corresponding MPI data type
typedef int MY_DT ; // data type for matrix

// #define MY_MPI_DT MPI_LONG_DOUBLE // corresponding MPI data type
// typedef long double MY_DT ; // data type for matrix


/***************************************************************************/
/* Global vars *************************************************************/
/***************************************************************************/
// MPI vars
int MY_MPI_RANK ;
int MY_MPI_SIZE ;

// Cartesian topology vars
int CART_COORDS[ 2 ] ;
int NDIMS = 2 ; // number of dimensions of cartesian grid, 2d for matmul
int MY_CART_RANK ;
MPI_Comm MY_CART_COMM ;

// Matrix related vars
int MAT_ROWS = 16 ;// Square matrix: cols = rows
int ROWS_PER_RANK ;
int ROWS_BLOCK ; // rows or cols for matrix with block as entry
int ENTRIES_PER_RANK ; // number of entries in matrices for each rank
MY_DT * LOCAL_MAT_A ;
MY_DT * LOCAL_MAT_B ;
MY_DT * LOCAL_MAT_C ;

// Others
int NUM_NODES ; // Var to denote number of nodes in BGQ
int NTASKS ; // Var to num of tasks per node 
char MY_FNAME[ 80 ] ; // Filename of binary file for matrix C


/***************************************************************************/
/* Function Decs ***********************************************************/
/***************************************************************************/
void SetParameters( char ** arg_vec ) ;
void PrintInitConditions() ;

MY_DT *InitializeMatrix( const char initialization_option , const int rows , const int cols ) ;
void SequentialMultiplication( int rows , MY_DT *matrix_A,MY_DT *matrix_B,MY_DT *matrix_C ) ;
void PrintMatrix(MY_DT *input_matrix,const int num_rows, const int num_cols) ;
void FreeMatrix() ;

void SetupCartTopology() ;
void InitializeAlignment() ;
void CirculateBlockMatmul( double * actual_time , double * overhead ) ;
void CannonMatmul() ;

void ParallelWrite( MPI_File * fh ) ;



/***************************************************************************/
/* Function: Main **********************************************************/
/***************************************************************************/

int main(int argc, char *argv[])
{
    // Define parameters for MPI I/O
    MPI_File * fh = malloc( sizeof( MPI_File ) ) ;

    // -----------------------------------------------------------------
    // Initialize MPI
    // -----------------------------------------------------------------
    MPI_Init( &argc, &argv);
    MPI_Comm_size( MPI_COMM_WORLD, & MY_MPI_SIZE );
    MPI_Comm_rank( MPI_COMM_WORLD, & MY_MPI_RANK );

    SetParameters( argv ) ;

    // -----------------------------------------------------------------
    // Initialize matrices A, B, C, and related size parameters 
    // -----------------------------------------------------------------
    ROWS_BLOCK = ( int ) sqrt( MY_MPI_SIZE ) ; // num of blocks = MY_MPI_SIZE
    ROWS_PER_RANK = MAT_ROWS / ROWS_BLOCK ;
    ENTRIES_PER_RANK = ROWS_PER_RANK * ROWS_PER_RANK ;

    LOCAL_MAT_A = InitializeMatrix( 'O' , ROWS_PER_RANK , ROWS_PER_RANK ) ;
    LOCAL_MAT_B = InitializeMatrix( 'O' , ROWS_PER_RANK , ROWS_PER_RANK ) ;
    LOCAL_MAT_C = InitializeMatrix( 'Z' , ROWS_PER_RANK , ROWS_PER_RANK ) ;


    // -----------------------------------------------------------------
    // Perform matrix multiplication via Cannon algorithm
    // -----------------------------------------------------------------
    CannonMatmul() ;

    // -----------------------------------------------------------------
    // Perform parallel I/O to write LOCAL_MAT_C to binary file 
    // -----------------------------------------------------------------
    ParallelWrite( fh )  ;

    // -----------------------------------------------------------------
    // Deallocate and finalize MPI
    // -----------------------------------------------------------------
    MPI_Barrier( MPI_COMM_WORLD ) ;
    free( fh ) ;
    FreeMatrix() ;
    MPI_Comm_free( & MY_CART_COMM ) ;
    MPI_Finalize();

    return EXIT_SUCCESS ;
}

// set size of matrix and fname, run ./cannon matrix_rows 
void SetParameters( char ** arg_vec ) {
    MAT_ROWS = atoi( arg_vec[ 1 ] ) ;
    NTASKS = atoi( arg_vec[ 2 ] ) ;
    NUM_NODES = MY_MPI_SIZE / NTASKS ;
    sprintf( MY_FNAME , "nds_%d_rks_%d_rows_%d.bin" , 
             NUM_NODES , MY_MPI_SIZE , MAT_ROWS ) ;
}

MY_DT *InitializeMatrix( const char initialization_option , const int rows , const int cols )
{
	
	int total_cell_num ;
    total_cell_num = rows * cols ;
	
	MY_DT *my_matrix = ( MY_DT * ) malloc( sizeof( MY_DT ) * total_cell_num ) ;
	
	int cell_i ;
	
	// Option 1 (Global Index): Dynamically allocate the matrix and initialize based on the global index of each cell
	if (initialization_option == 'G')
	{
        //int global_ini = MY_MPI_RANK * total_cell_num ;
		for (cell_i = 0 ; cell_i < total_cell_num ; cell_i++)
		{
			//my_matrix[cell_i] = global_ini + cell_i;
			my_matrix[cell_i] = MY_MPI_RANK ;
		}
	}
	// Option 2 (All Ones): Dynamically allocate the matrix and initialize to ones 
	else if (initialization_option == 'O')
	{
		for (cell_i = 0 ; cell_i < total_cell_num ; cell_i++)
		{
			my_matrix[cell_i] = 1;
		}
	}
	// Option 3 (All Zeros): Dynamically allocate the matrix and initialize to zeros 
	else if (initialization_option == 'Z')
	{
		for (cell_i = 0 ; cell_i < total_cell_num ; cell_i++)
		{
			my_matrix[cell_i] = 0;
		}
	}
	else
	{
		printf("ERROR: The initialization option can only be G (global index) or O (all ones) or Z (all zeros)! \n");
        exit( EXIT_FAILURE ) ;
	}
	
	return my_matrix;
}

void SequentialMultiplication( int rows , MY_DT *matrix_A,MY_DT *matrix_B,MY_DT *matrix_C )
{
	int i_serial, j_serial, k_serial ;
	for(i_serial = 0 ; i_serial < rows ; i_serial++)
	{
		for(j_serial = 0 ; j_serial < rows ; j_serial++)
		{
			for(k_serial = 0 ; k_serial < rows ; k_serial++)
			{
				//calculate C = A*B
				matrix_C[i_serial * rows + j_serial] += matrix_A[i_serial * rows + k_serial] * matrix_B[k_serial * rows + j_serial];  
			}
		}
	}
}

void CannonMatmul() {

#ifdef BGQ
    unsigned long long start_cycles = 0;
    unsigned long long end_cycles = 0;
#else
    double start_cycles = 0;
    double end_cycles = 0;
#endif

    double time_in_secs ;

    // Record start time 
    MPI_Barrier( MPI_COMM_WORLD );
    if (0 == MY_MPI_RANK ) {
        PrintInitConditions() ;
        start_cycles = GetTimeBase();
    }

    // Create cartesian topology, init rank and coordiates in new cart comm
    SetupCartTopology() ;

    // Initial alignment
    InitializeAlignment() ;

    // Circulate blocks and perform block matrix multiplication 
    double actual_matmul_time , overhead_time ;
    CirculateBlockMatmul( & actual_matmul_time , & overhead_time ) ;
    if (0 == MY_MPI_RANK ) {
        printf( "# Real matmul time:\n\t%f\n", actual_matmul_time ) ;
        printf( "# Isend and Irecv time:\n\t%f\n", overhead_time ) ;
    }

    // Record end time 
    MPI_Barrier( MPI_COMM_WORLD );
    if (0 == MY_MPI_RANK ) {
        end_cycles = GetTimeBase() ;
        time_in_secs = ((double)( end_cycles - start_cycles ) ) / PROC_FREQ ;
        printf( "# Cannon total execution time:\n\t%f\n", time_in_secs ) ;
    }
}

void SetupCartTopology() {

#ifdef BGQ
    unsigned long long start_setup_cart = 0;
    unsigned long long end_setup_cart = 0;
#else
    double start_setup_cart = 0;
    double end_setup_cart = 0;
#endif
    double time_in_secs ;

    if (0 == MY_MPI_RANK ) { start_setup_cart = GetTimeBase(); }

    // Create cartesian topology
    int cart_dims[ NDIMS ] ;
    cart_dims[ 0 ] = cart_dims[ 1 ] = ROWS_BLOCK ; 
    int cart_periods[] = { TRUE , TRUE } ;
    MPI_Cart_create( MPI_COMM_WORLD , NDIMS , cart_dims , cart_periods , 1 , & MY_CART_COMM ) ; 

    // Initialize rank and coordiates in new Cartesian comm
    MPI_Comm_rank( MY_CART_COMM , & MY_CART_RANK ) ;
    MPI_Cart_coords( MY_CART_COMM , MY_CART_RANK , NDIMS , CART_COORDS ) ;

    if (0 == MY_MPI_RANK ) {
        end_setup_cart = GetTimeBase() ;
        time_in_secs = ((double)( end_setup_cart - start_setup_cart ) ) / PROC_FREQ ;
        printf( "# SetupCartTopology() takes time:\n\t%f\n", time_in_secs ) ;
    }
} 

void InitializeAlignment() {
#ifdef BGQ
    unsigned long long start_init_align = 0;
    unsigned long long end_init_align = 0;
#else
    double start_init_align = 0;
    double end_init_align = 0;
#endif

    double time_in_secs ;

    if (0 == MY_MPI_RANK ) { start_init_align = GetTimeBase(); }
    int shift_src_a , shift_dst_a , shift_src_b , shift_dst_b ;
    MPI_Status status ;

    // For A, no shift for row 0, 1 left shift for row 1, 2 left shift for row 2, etc.
    MPI_Cart_shift( MY_CART_COMM , 0 , -CART_COORDS[ 1 ] , & shift_src_a , & shift_dst_a ) ;

    // For B, no shift for col 0, 1 up shift for col 1, 2 up shift for col 2, etc.
    MPI_Cart_shift( MY_CART_COMM , 1 , -CART_COORDS[ 0 ] , & shift_src_b , & shift_dst_b ) ;

    MPI_Sendrecv_replace( LOCAL_MAT_A , ENTRIES_PER_RANK , MY_MPI_DT , shift_dst_a , 
                          1 , shift_src_a , 1 , MY_CART_COMM , & status ) ; 

    MPI_Sendrecv_replace( LOCAL_MAT_B , ENTRIES_PER_RANK , MY_MPI_DT , shift_dst_b , 
                          1 , shift_src_b , 1 , MY_CART_COMM , & status ) ; 

    MPI_Barrier( MPI_COMM_WORLD ) ;

    if (0 == MY_MPI_RANK ) {
        end_init_align = GetTimeBase() ;
        time_in_secs = ((double)( end_init_align - start_init_align ) ) / PROC_FREQ ;
        printf( "# InitializeAlignment() takes time:\n\t%f\n", time_in_secs ) ;
    }
}

// -------------------------------------------------------------------------------------------
// Circulate blocks and perform block matrix multiplication 
// A - circulated from right to left, B - circulated from down to up
// -------------------------------------------------------------------------------------------
void CirculateBlockMatmul( double * real , double * overhead ) {
#ifdef BGQ
    unsigned long long start = 0;
    unsigned long long end = 0;
#else
    double start = 0 ;
    double end = 0 ;
#endif

    * real = 0 ;
    * overhead = 0 ;
    
    // Obtain ranks of the left and top shifts
    int left_rank , right_rank , top_rank , bottom_rank ;
    MPI_Cart_shift( MY_CART_COMM , 0 , -1 , & right_rank , & left_rank ) ;
    MPI_Cart_shift( MY_CART_COMM , 1 , -1 , & bottom_rank , & top_rank ) ;

    MPI_Status status ;
    int i ;
    int j ;

    MY_DT *buff_a[ 2 ] , *buff_b[ 2 ] ; 
    buff_a[ 0 ] = LOCAL_MAT_A ;
    buff_a[ 1 ] = ( MY_DT * ) malloc( ENTRIES_PER_RANK * sizeof( MY_DT ) ) ;
    buff_b[ 0 ] = LOCAL_MAT_B ;
    buff_b[ 1 ] = ( MY_DT * ) malloc( ENTRIES_PER_RANK * sizeof( MY_DT ) ) ;
    MPI_Request reqts[ 4 ] ;

    for( i = 0 ; i < ROWS_BLOCK ; i++ ) {

        if (0 == MY_MPI_RANK ) { start = GetTimeBase(); }

        MPI_Isend( buff_a[ i % 2 ] , ENTRIES_PER_RANK , MY_MPI_DT , left_rank , 1 ,
                MY_CART_COMM , &reqts[ 0 ] ) ;

        MPI_Isend( buff_b[ i % 2 ] , ENTRIES_PER_RANK , MY_MPI_DT , top_rank , 1 ,
                MY_CART_COMM , &reqts[ 1 ] ) ;

        MPI_Irecv( buff_a[ ( i + 1 ) % 2 ] , ENTRIES_PER_RANK , MY_MPI_DT , right_rank , 1 ,
                MY_CART_COMM , &reqts[ 2 ] ) ;

        MPI_Irecv( buff_b[ ( i + 1 ) % 2 ] , ENTRIES_PER_RANK , MY_MPI_DT , bottom_rank , 1 ,
                MY_CART_COMM , &reqts[ 3 ] ) ;

        if (0 == MY_MPI_RANK ) { end = GetTimeBase(); }
        if (0 == MY_MPI_RANK ) { * overhead +=  ((double)( end - start ) ) / PROC_FREQ ;} 

        if (0 == MY_MPI_RANK ) { start = GetTimeBase(); }
        SequentialMultiplication( ROWS_PER_RANK , buff_a[ i % 2 ], buff_b[ i % 2 ] , LOCAL_MAT_C ) ;
        if (0 == MY_MPI_RANK ) { end = GetTimeBase(); }
        if (0 == MY_MPI_RANK ) { * real +=  ((double)( end - start ) ) / PROC_FREQ ; } 
        

        if (0 == MY_MPI_RANK ) { start = GetTimeBase(); }
        for( j = 0 ; j < 4 ; j++ ) {
            MPI_Wait( &reqts[ j ] , &status ) ;
        }
        if (0 == MY_MPI_RANK ) { end = GetTimeBase(); }
        if (0 == MY_MPI_RANK ) { * overhead +=  ((double)( end - start ) ) / PROC_FREQ ; } 

    }

    free( buff_a[ 1 ] ) ;
    free( buff_b[ 1 ] ) ;

}

void ParallelWrite( MPI_File * fh ) {

    MPI_Offset offset ;
    MPI_Status status;

    // Variables to compute offset
    int blk_row = MY_MPI_RANK % ROWS_BLOCK ; // block row index 
    int blk_col = MY_MPI_RANK / ROWS_BLOCK ;
    int glb_row = blk_row * ROWS_PER_RANK ; // entry row index at global matrix C
    int glb_col = blk_col * ROWS_PER_RANK ;

    offset = ( glb_row * MAT_ROWS + glb_col ) * sizeof( MY_DT ) ; // Offset of first entry of local mat C in global matrix C
    MPI_File_open( MPI_COMM_WORLD, MY_FNAME , MPI_MODE_RDWR | MPI_MODE_CREATE , MPI_INFO_NULL, fh ) ;


#ifdef BGQ
    unsigned long long start_cycles = 0;
    unsigned long long end_cycles = 0;
#else
    double start_cycles = 0;
    double end_cycles = 0;
#endif

    double time_in_secs ;
    int i ;

    // Record start time 
    MPI_Barrier( MPI_COMM_WORLD );
    if (0 == MY_MPI_RANK ) {
        start_cycles = GetTimeBase();
    }

    for( i = 0 ; i < ROWS_PER_RANK ; i++ ) {
        MPI_File_write_at_all( *fh , offset , LOCAL_MAT_C + i * ROWS_PER_RANK , ROWS_PER_RANK , MY_MPI_DT , &status );
        offset += ( MAT_ROWS - ROWS_PER_RANK ) * sizeof( MY_DT ) ;
    }

    // Record end time 
    MPI_Barrier( MPI_COMM_WORLD );
    if (0 == MY_MPI_RANK ) {
        end_cycles = GetTimeBase() ;
        time_in_secs = ((double)( end_cycles - start_cycles ) ) / PROC_FREQ ;
        printf( "#Parallel I/O time:\n\t%f\n", time_in_secs ) ;
    }

    MPI_File_close( fh ); 
}

void PrintMatrix(MY_DT *input_matrix,const int num_rows, const int num_cols)
{
	int row_i_print, col_j_print;
	int global_index  = 0;
	for(row_i_print=0;row_i_print<num_rows;row_i_print++)
	{
		for(col_j_print=0;col_j_print<num_cols;col_j_print++)
		{
            if( MY_MPI_DT == MPI_LONG_DOUBLE )
                printf("%Lf\t",input_matrix[global_index]);
            else 
                printf("%d\t",input_matrix[global_index]);

			global_index++;
		}    
		printf("\n");
	}
	printf("\n");
	printf("\n");
}

void FreeMatrix()
{
	free( LOCAL_MAT_A );
	free( LOCAL_MAT_B );
	free( LOCAL_MAT_C );
}

// Print key parameters
void PrintInitConditions() {
        printf( "#Initial Conditions:\n" ) ;
        printf( "#\tDATATYPE_SIZE = %lu\n" , sizeof( MY_DT ) ) ;
        printf( "#\tNUM_NODES = %d\n" , NUM_NODES ) ;
        printf( "#\tNUM_RNAKS = %d\n" , MY_MPI_SIZE ) ;
        printf( "#\tMAT_ROWS = %d\n\n" , MAT_ROWS ) ;
}
