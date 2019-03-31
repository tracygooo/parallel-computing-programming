#include<stdio.h> 
#include<stdlib.h>

#define ALIVE 1
#define GRID_SIZE 8

int** Allocation(int input_rows, int input_cols) ;
void Initialization(int **input_grid_chunk, int input_rows, int input_cols) ;
void PrintGrid(int **input_grid_chunk, int start_row, int end_row) ;
void PrintGridAddress(int **input_grid_chunk, int start_row, int end_row) ;

int main( int argc , char ** argv )
{
    int **local_universe = Allocation ( 8 ,8 );
    Initialization(local_universe, 8 , 8 );
    PrintGrid( local_universe , 0 , 7 ) ;
    PrintGridAddress( local_universe , 0 , 7 ) ;

    // print out address of first item in each row
    int i ;
    for( i = 0 ; i < 8 ; i++ ){
        printf( "row %d: %d\n", i , local_universe[ i ] ) ;
    }
}


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
