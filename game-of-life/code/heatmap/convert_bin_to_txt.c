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

// HEATMAP_SIZE = GRID_SIZE / HEATMAP_CHUNK_SIZE
#define HEATMAP_SIZE 1024 
// #define HEATMAP_SIZE 8 

//=================================================================================
// Function declarations
//=================================================================================
int** Allocation(int input_rows, int input_cols) ;
void PrintHeatmap( int ** heatmap , const int hm_size ) ;
void FreeGrid(int **input_grid_chunk, int input_rows) ;
void InitializeArray( int ** array , const int rows, const int cols ) ;
void FprintHeatmap( const char * fname , int ** heatmap , const int hm_size ) ;

int main( int argc , char ** argv ) {
    // -------------------------------------------------------------------
    // Set parameters
    // -------------------------------------------------------------------
    int nodes = atoi( argv[ 1 ] ) ;
    int threads_per_rank = atoi( argv[ 2 ] ) ;
    double random_threshold = atof( argv[ 3 ] ) ;
    int ticks = atoi( argv[ 4 ] ) ;

    // -------------------------------------------------------------------
    // Define name of read and write files 
    // -------------------------------------------------------------------
    char rd_fname[ 80 ] ;
    char wt_fname[ 80 ] ;
    sprintf( rd_fname , "heatmap_nds_%d_tpr_%d_thh_%f_tks_%d.bin" , 
             nodes , threads_per_rank , random_threshold , ticks) ;
    sprintf( wt_fname , "heatmap_nds_%d_tpr_%d_thh_%f_tks_%d.txt" , 
             nodes , threads_per_rank , random_threshold , ticks) ;

    // -------------------------------------------------------------------
    // Read file in 
    // -------------------------------------------------------------------
    int ** heatmap = Allocation( HEATMAP_SIZE , HEATMAP_SIZE ) ;
    FILE * fp ; 
    fp = fopen( rd_fname , "rb+" ) ;
    int i ;
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

    // -------------------------------------------------------------------
    // Write to text file 
    // -------------------------------------------------------------------
    FprintHeatmap( wt_fname , heatmap , HEATMAP_SIZE ) ;

    // Free dynamic memory
    FreeGrid( heatmap , HEATMAP_SIZE ) ;

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

void FprintHeatmap( const char * fname , int ** heatmap , const int hm_size )
{
    FILE *fp = fopen( fname , "w" ) ;

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
