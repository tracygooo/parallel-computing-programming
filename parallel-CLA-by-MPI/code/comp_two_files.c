/*
 * Compare two files with one long line in each file 
 * Output the postions of characters and total number of difference
 * Run ./a.out file_name1 file_name2
 * Jinghua Feng, tracygooo@gmail.com, Feb 15 2019
 */

#include <stdio.h>
#include <stdlib.h>

// number of characters in each line
#define MY_INPUT_SIZE 262144

int ReadInput( const char * fname1 , const char * fname2 ,char * hex_input_a , char * hex_input_b ) ;

int main( int argc , char ** argv ) {

    char * line1 = ( char * ) malloc( MY_INPUT_SIZE + 1 ) ;
    char * line2 = ( char * ) malloc( MY_INPUT_SIZE + 1 ) ;
    ReadInput( argv[ 1 ] ,argv[ 2 ] , line1 , line2 ) ;

    int i ;
    int diff_cnt  = 0 ;
    for( i = 0 ; i < MY_INPUT_SIZE ; i++ ) {
        if( line1[ i ] != line2[ i ] ){
            printf( "%d\t" , i + 1 ) ;
            diff_cnt++ ;
        }
    }

    printf( "\n" ) ;
    printf( "Total number of differences is %d\n" , diff_cnt ) ;

    free( line1 ) ;
    free( line2 ) ;

    return EXIT_SUCCESS ;
}

int ReadInput( const char * fname1 , const char * fname2 ,char * hex_input_a , char * hex_input_b ) { 

    FILE * file1 ;
    FILE * file2 ;
    if( ( file1 = fopen( fname1 , "r" ) ) == NULL ) {
        printf( "Failed to open input data file: %s\n" , fname1 ) ;
        exit( EXIT_FAILURE ) ;
    }

    if( ( file2 = fopen( fname2 , "r" ) ) == NULL ) {
        printf( "Failed to open input data file: %s\n" , fname2 ) ;
        exit( EXIT_FAILURE ) ;
    }

    fscanf( file1 , "%s" , hex_input_a ) ;
    fscanf( file2 , "%s" , hex_input_b ) ;


    // printf( "hex1:\n%s\n\n" , HEX_INPUT_A ) ;
    // printf( "hex2:\n%s\n\n" , HEX_INPUT_B ) ;
    fclose( file1 ) ;
    fclose( file2 ) ;

    return EXIT_SUCCESS ;
}
