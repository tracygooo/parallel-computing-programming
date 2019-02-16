/*
 * C program to parallel the sum of two 1,048,576-bit numbers by uing MPI (message passing interface)
 * Compute speedup of parallel MPI CLA relative to serial MPI CLA 
 * Compute speedup of parallel MPI CLA relative to serial RCA(ripple carry adder)
 * Jinghua Feng, fengj3@rpi.edu
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// EXAMPLE DATA STRUCTURE DESIGN AND LAYOUT FOR CLA
#define MY_INPUT_SIZE 262144
//#define MY_INPUT_SIZE 400 
#define BITS MY_INPUT_SIZE * 4
#define BLOCK_SIZE 32 

int RippleCarryAdder( const int * gi , const int * pi , const int bits , int * sumi ) ;
int ComputeGiPi( const int * bin1 , const int * bin2 , const int bits , const int rank , int * gi , int * pi ) ;

int main( int argc , char ** argv ) {

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
    RippleCarryAdder( BIN1 , BIN2 , BITS , SUMI ) ; 
    free( BIN1 ) ;
    free( BIN2 ) ;
    HEX_OUTPUT = ( char * ) malloc( MY_INPUT_SIZE + 1 ) ;

    // Convert binary sumi to hex form 
    ConvertBinaryToHex( SUMI , BITS  , HEX_OUTPUT ) ;
    WriteOutput( argv[ 2 ] , HEX_OUTPUT ) ;

    free( SUMI ) ;
    free( HEX_OUTPUT ) ;

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

