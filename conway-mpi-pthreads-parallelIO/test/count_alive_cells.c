#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<math.h>

//#include<clcg4.h>

//#include<mpi.h>
//#include<pthread.h>

#define ALIVE 1
#define DEAD 0 
#define GRID_SIZE 4 
#define mpi_commsize 1 

int CountAdjacentAliveCells(int **input_grid_chunk, const int cell_row_index, const int cell_col_index);
int** Allocation(int input_rows, int input_cols);
void UpdateCellWithBasicRules(int **input_grid_chunk, int start_row, int end_row);
void PrintGrid(int **input_grid_chunk, int start_row, int end_row) ;

int main(int argc, char *argv[])
{
    int rows = GRID_SIZE + 2 ;
    int cols = GRID_SIZE ;
    //int my_array[ rows ][ cols ] ;
    int** my_array = Allocation( rows, cols ) ; 

    int i , j ;
    for( i = 0 ; i < rows ; i++ )
    {
        for( j = 0 ; j < cols ; j++ )
        {
            if( ( ( j == 1 ) || ( j == 3 ) ) && i == 1 )
                my_array[ i ][ j ] = 1 ;
            else
                my_array[ i ][ j ] = 0 ;
            /*
            if( i > 1 && i < 4  )
                my_array[ i ][ j ] = 1 ;
            else
                my_array[ i ][ j ] = 0 ;
                */

            printf( "%d " , my_array[ i ][ j ] ) ;
        }
        printf( "\n" ) ;
    }
    printf( "\n" ) ;

    
    for( i = 1 ; i < GRID_SIZE + 1 ; i++ )
    {
        for( j = 0 ; j < GRID_SIZE; j++ )
        {
            printf( "%d " , CountAdjacentAliveCells( my_array , i , j ) ) ;
        }
        printf( "\n" ) ;
    }
    printf( "Before Update!!!\n" ) ;
    PrintGrid( my_array , 0 , GRID_SIZE + 1 ) ;

    UpdateCellWithBasicRules( my_array , 1 , GRID_SIZE  );
    printf( "After Update!!!\n" ) ;
    PrintGrid( my_array , 0 , GRID_SIZE + 1 ) ;

    return EXIT_SUCCESS ;
}

int CountAdjacentAliveCells(int **input_grid_chunk, const int cell_row_index, const int cell_col_index)
{
    int number_alive_neighbors = 0;
    //There are 8 neighbors for each given cell_col_index
    int i, j;

    if (cell_row_index<1 || cell_row_index > GRID_SIZE / mpi_commsize )
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

void UpdateCellWithBasicRules(int **input_grid_chunk, int start_row, int end_row)
{
    int i, j;
    for(i = start_row; i < end_row+1; i++)
    {
        for(j = 0; j < GRID_SIZE; j++)
        {
            int my_alive_neighbor_num = CountAdjacentAliveCells(input_grid_chunk, i,j);

            // ALIVE, < 2 live neighbor, changed to be DEAD
            if ( input_grid_chunk[i][j] == (int)ALIVE && my_alive_neighbor_num < 2 )
            {
                input_grid_chunk[i][j] = (int)DEAD;
            }

            // ALIVE, 2 or 3 live neighbors, still ALIVE, no need to change, skip
            
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
