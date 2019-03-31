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
void InitializeArray( int ** array , const int rows, const int cols ) ;
void FprintHeatmap( int ** heatmap , const int hm_size ) ;

int main( int argc , char ** arcv ) {
    int ** heatmap = Allocation( HEATMAP_SIZE , HEATMAP_SIZE ) ;
    FILE * fp ; 
    //fp = fopen( "heatmap.bin" , "rb+" ) ;

    //----------------------------create test bin file----------------------------
    fp = fopen( "test.bin" , "wb" ) ;
    int ** write_array = Allocation( HEATMAP_SIZE , HEATMAP_SIZE ) ;
    InitializeArray( write_array , HEATMAP_SIZE , HEATMAP_SIZE ) ;
    PrintHeatmap( write_array , HEATMAP_SIZE ) ;
    int i ;
    for( i = 0 ; i < HEATMAP_SIZE ; i++ ) {
        fwrite( write_array[ i ] , sizeof( int ) , HEATMAP_SIZE , fp ) ;
    }
    FreeGrid( write_array , HEATMAP_SIZE ) ;
    fclose( fp ) ;
    //--------------------------------------------------------------------------

    //int i ;
    fp = fopen( "test.bin" , "rb" ) ;
    size_t read_size ;
    for( i = 0 ; i < HEATMAP_SIZE ; i++ ) {
        read_size = fread( heatmap[ i ] , sizeof(int) , HEATMAP_SIZE , fp ) ;
        if( read_size != HEATMAP_SIZE ) {
            if( feof( fp ) )
                printf("Error reading heatmap.bin: unexpected end of file\n");
            else{
                perror( "Error reading heatmap.bin" ) ;
                exit( EXIT_FAILURE ) ;
            }

        }
    }

    fclose( fp ) ;

    PrintHeatmap( heatmap , HEATMAP_SIZE ) ;
    FprintHeatmap( heatmap , HEATMAP_SIZE ) ;
    FreeGrid( heatmap , HEATMAP_SIZE ) ;

    system( "gnuplot -p 'heatmap.gp'") ;

    return EXIT_SUCCESS ;
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

void InitializeArray( int ** array , const int rows, const int cols ){
    int i , j ;
    for( i = 0 ; i < rows ; i++ ) {
        for( j = 0 ; j < cols ; j++ ) {
            array[ i ][ j ] = i + j ;
        }
    }
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

void FprintHeatmap( int ** heatmap , const int hm_size )
{
    FILE *fp = fopen( "heatmap.txt" , "w" ) ;

    int i, j;
    for(i = 0 ; i < hm_size ; i++)
    {
        for(j = 0; j < hm_size ; j++)
        {
            fprintf(fp , "%d ", heatmap[ i ][ j ] ) ;
        }

        fprintf( fp , "\n" ) ;
    }

    fclose( fp ) ;
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
