/*
 * Use ripple carrier adder to sum two numbers sequentially 
 * Jinghua Feng, fengj3@rpi.edu
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define MY_INPUT_SIZE 262144
#define BITS MY_INPUT_SIZE * 4
#define BLOCK_SIZE 32 

int RippleCarryAdder( const int * gi , const int * pi , const int bits , int * sumi ) ;
int ReadInput( const char * fname  , char * hex_input_a , char * hex_input_b ) ;
int WriteOutput( const char * fname , const char * hex_output ) ;
int ComputeGiPi( const int * bin1 , const int * bin2 , const int bits , const int rank , int * gi , int * pi ) ;
int ConvertHexToBinary( const char * hex , int * bin ) ;
int ConvertBinaryToHex( const int * sumi , const int bits , char * hex_output ) ;

int main( int argc , char ** argv ) {

    int my_mpi_size = -1 ;
    int my_mpi_rank = -1 ;

    MPI_Init( &argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &my_mpi_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_mpi_rank);

    double start_time , finish_time ;

    // Read input file to HEX_INPUT_A and HEX_INPUT_B 
    char * HEX_INPUT_A = ( char * ) malloc( MY_INPUT_SIZE + 1 ) ;
    char * HEX_INPUT_B = ( char * ) malloc( MY_INPUT_SIZE + 1 ) ;
    ReadInput( argv[ 1 ] , HEX_INPUT_A , HEX_INPUT_B ) ;

    // Convert hex to binary and revert binary 
    int * BIN1 = ( int * ) malloc( BITS * sizeof( int ) );
    int * BIN2 = ( int * ) malloc( BITS * sizeof( int ) );
    ConvertHexToBinary( HEX_INPUT_A , BIN1 ) ;
    ConvertHexToBinary( HEX_INPUT_B , BIN2 ) ;
    free( HEX_INPUT_A ) ;
    free( HEX_INPUT_B ) ;
    //PrintArray( BIN1 , BITS , "bin1" ) ; 
    //PrintArray( BIN2 , BITS , "bin2" ) ; 

    int * SUMI = ( int * ) malloc( BITS * sizeof( int ) );

    start_time = MPI_Wtime() ;
    RippleCarryAdder( BIN1 , BIN2 , BITS , SUMI ) ; 
    finish_time = MPI_Wtime() ;
    printf( "Ripple execution time: %lf\n" ,finish_time - start_time ) ;

    free( BIN1 ) ;
    free( BIN2 ) ;
    char * HEX_OUTPUT = ( char * ) malloc( MY_INPUT_SIZE + 1 ) ;

    // Convert binary sumi to hex form 
    ConvertBinaryToHex( SUMI , BITS  , HEX_OUTPUT ) ;
    WriteOutput( argv[ 2 ] , HEX_OUTPUT ) ;

    free( SUMI ) ;
    free( HEX_OUTPUT ) ;

    MPI_Finalize();

    return EXIT_SUCCESS ;
}

/*****************************************
 * Serial ripple alorithm 
 ****************************************/
int RippleCarryAdder( const int * bin1 , const int * bin2 , const int bits , int * sumi ) {
    int i ;

    // Compute gi and pi
    int * gi = ( int * ) malloc( bits * sizeof( int ) ) ;
    int * pi = ( int * ) malloc( bits * sizeof( int ) ) ;
    for( i = 0 ; i < bits ; i++ ) {
        //printf( "gi[%d] = %d" , i , gi[ i ]) ;
        gi[ i ] = bin1[ i ] && bin2[ i ] ;
        pi[ i ] = bin1[ i ] || bin2[ i ] ; 
    }

    // Get carry-in array ci
    int * ci = ( int * ) malloc( bits * sizeof( int ) ) ;
    for( i = 0 ; i < bits ; i++ ) {
        if( i == 0 ) ci[ i ] = gi[ i ] || ( pi[ i ] && 0 ) ;
        else ci[ i ] = gi[ i ] || ( pi[ i ] && ci[ i -1 ] ) ;
    }

    // Compute sumi
    for( i = 0 ; i < bits ; i++ ) {
        if( i == 0 ) sumi[ i ] = bin1[ i ] ^ bin2[ i ] ^ 0 ;
        else sumi[ i ] = bin1[ i ] ^ bin2[ i ] ^ ci[ i - 1 ] ;
    }

    // PrintArray( ci , BITS , "ripple ci" ) ;
    // PrintArray( sumi , BITS , "ripple sumi" ) ;
    
    free( gi ) ;
    free( pi ) ;
    free( ci ) ;

    return EXIT_SUCCESS ;
}

int ReadInput( const char * fname  , char * hex_input_a , char * hex_input_b ) { 

    FILE * file ;
    if( ( file = fopen( fname , "r" ) ) == NULL ) {
        printf( "Failed to open input data file: %s\n" , fname ) ;
        exit( EXIT_FAILURE ) ;
    }
    fscanf( file , "%s %s" , hex_input_a , hex_input_b ) ;

    // printf( "hex1:\n%s\n\n" , HEX_INPUT_A ) ;
    // printf( "hex2:\n%s\n\n" , HEX_INPUT_B ) ;
    fclose( file ) ;

    return EXIT_SUCCESS ;
}

int WriteOutput( const char * fname , const char * hex_output ) { 

    FILE * file ;
    if( ( file = fopen( fname , "w" ) ) == NULL ) {
        printf( "Failed to open output data file: %s\n" , fname ) ;
        exit( EXIT_FAILURE ) ;
    }
    fprintf( file , "%s\n" , hex_output ) ;
    fclose( file ) ;

    return EXIT_SUCCESS ;
}

int ConvertHexToBinary( const char * hex , int * bin ) {
    int i ;
    int j ;
    j = BITS - 1 ;

    for( i = 0 ; i < MY_INPUT_SIZE ; i++ ) {
        switch( hex[ i ] ) {
            case '0' :
                bin[ j-- ] = 0 ; bin[ j-- ] = 0 ; bin[ j-- ] = 0 ; bin[ j-- ] = 0 ;
                break ;
            case '1' :
                bin[ j-- ] = 0 ; bin[ j-- ] = 0 ; bin[ j-- ] = 0 ; bin[ j-- ] = 1 ;
                break ;
            case '2' :
                bin[ j-- ] = 0 ; bin[ j-- ] = 0 ; bin[ j-- ] = 1 ; bin[ j-- ] = 0 ;
                break ;
            case '3' :
                bin[ j-- ] = 0 ; bin[ j-- ] = 0 ; bin[ j-- ] = 1 ; bin[ j-- ] = 1 ;
                break ;
            case '4' :
                bin[ j-- ] = 0 ; bin[ j-- ] = 1 ; bin[ j-- ] = 0 ; bin[ j-- ] = 0 ;
                break ;
            case '5' :
                bin[ j-- ] = 0 ; bin[ j-- ] = 1 ; bin[ j-- ] = 0 ; bin[ j-- ] = 1 ;
                break ;
            case '6' :
                bin[ j-- ] = 0 ; bin[ j-- ] = 1 ; bin[ j-- ] = 1 ; bin[ j-- ] = 0 ;
                break ;
            case '7' :
                bin[ j-- ] = 0 ; bin[ j-- ] = 1 ; bin[ j-- ] = 1 ; bin[ j-- ] = 1 ;
                break ;
            case '8' :
                bin[ j-- ] = 1 ; bin[ j-- ] = 0 ; bin[ j-- ] = 0 ; bin[ j-- ] = 0 ;
                break ;
            case '9' :
                bin[ j-- ] = 1 ; bin[ j-- ] = 0 ; bin[ j-- ] = 0 ; bin[ j-- ] = 1 ;
                break ;
            case 'A' :
                bin[ j-- ] = 1 ; bin[ j-- ] = 0 ; bin[ j-- ] = 1 ; bin[ j-- ] = 0 ;
                break ;
            case 'B' :
                bin[ j-- ] = 1 ; bin[ j-- ] = 0 ; bin[ j-- ] = 1 ; bin[ j-- ] = 1 ;
                break ;
            case 'C' :
                bin[ j-- ] = 1 ; bin[ j-- ] = 1 ; bin[ j-- ] = 0 ; bin[ j-- ] = 0 ;
                break ;
            case 'D' :
                bin[ j-- ] = 1 ; bin[ j-- ] = 1 ; bin[ j-- ] = 0 ; bin[ j-- ] = 1 ;
                break ;
            case 'E' :
                bin[ j-- ] = 1 ; bin[ j-- ] = 1 ; bin[ j-- ] = 1 ; bin[ j-- ] = 0 ;
                break ;
            case 'F' :
                bin[ j-- ] = 1 ; bin[ j-- ] = 1 ; bin[ j-- ] = 1 ; bin[ j-- ] = 1 ;
                break ;
            default :
                printf( "Invalid hex number...\n" ) ;
                return EXIT_FAILURE ;
        }
    }

    return EXIT_SUCCESS ;
}

int ConvertBinaryToHex( const int * sumi , const int bits , char * hex_output ) {

    /*
     * i: iter for sumi
     * j: iter for hex_output (final output) 
     * my_sum: sum of every four bits 
     */
    int i , j , my_sum ;
    i = BITS - 1 ;
    j = 0 ;

    while( i >= 3 ) 
    {
        // Compute decimal of every four bits 
        my_sum = 0 ;
        my_sum += sumi[ i ] * 2 * 2 * 2 ;
        i-- ;
        my_sum += sumi[ i ] * 2 * 2 ;
        i-- ;
        my_sum += sumi[ i ] * 2 ;
        i-- ;
        my_sum += sumi[ i ] ;
        i-- ;

        if ( my_sum <= 9 ) hex_output[ j ] = my_sum  + '0' ;
        else 
        {
            switch( my_sum ) {
                case 10 :
                    hex_output[ j ] = 'A' ; break ;
                case 11 :
                    hex_output[ j ] = 'B' ; break ;
                case 12 :
                    hex_output[ j ] = 'C' ; break ;
                case 13 :
                    hex_output[ j ] = 'D' ; break ;
                case 14 :
                    hex_output[ j ] = 'E' ; break ;
                case 15 :
                    hex_output[ j ] = 'F' ; break ;
                default :
                    printf( "Invalid hex number...\n" ) ;
                    return EXIT_FAILURE ;
            }
        }
        
        j++ ;
    }

    // printf( "Addition:\n%s\n\n" , hex_output ) ;
    hex_output[ MY_INPUT_SIZE ] = '\0' ; 
    //printf( "%s\n" , hex_output ) ;
    return EXIT_SUCCESS ;
}
