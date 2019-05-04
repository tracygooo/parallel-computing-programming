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