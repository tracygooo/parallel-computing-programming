/*
 * Read from "heatmap.bin"
 * Write to "heatmap.txt"
 * Use gnuplot to draw heatmap 
 *
 */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<math.h>

// #define HEATMAP_SIZE 1024 
#define HEATMAP_SIZE 8 

//=================================================================================
// Function declarations
//=================================================================================
int** Allocation(int input_rows, int input_cols) ;
void PrintHeatmap( int ** heatmap , const int hm_size ) ;
void FreeGrid(int **input_grid_chunk, int input_rows) ;

int main( int argc , char ** arcv ) {
    int ** heatmap = Allocation( HEATMAP_SIZE , HEATMAP_SIZE ) ;
    FILE * f ; 
    f = fopen( "heatmap.bin" , "rb" ) ;
    int i ;
    for( i = 0 ; i < HEATMAP_SIZE ; i++ ) {
        fread( heatmap[ i ] , sizeof(int) , HEATMAP_SIZE , f ) ;
    }

    PrintHeatmap( heatmap , HEATMAP_SIZE ) ;
    FreeGrid( heatmap , HEATMAP_SIZE ) ;
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

void PrintHeatmap( int ** heatmap , const int hm_size )
{ 
    int i, j;
    printf("\n");
    for(i = 0 ; i < hm_size ; i++)
    {
        for(j = 0; j < hm_size ; j++)
        {
            printf("%d ",heatmap[ i ][ j ] ) ;
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
