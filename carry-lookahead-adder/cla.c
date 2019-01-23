/*
 * C program for a carry-lookahead adder
 * Simulate a 4,096 bit Carry lookahead adder by using 8-bit blocks
 * Jinghua Feng, fengj3@rpi.edu
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// EXAMPLE DATA STRUCTURE DESIGN AND LAYOUT FOR CLA
#define input_size 1024
#define block_size 8

//Do not touch these defines
#define digits (input_size+1)
#define bits digits*4
#define ngroups bits/block_size
#define nsections ngroups/block_size
#define nsupersections nsections/block_size

//Global definitions of the various arrays used in steps for easy access
int gi[bits] = {0};
int pi[bits] = {0};
int ci[bits] = {0};
int ggj[ngroups] = {0};
int gpj[ngroups] = {0};
int gcj[ngroups] = {0};
int sgk[nsections] = {0};
int spk[nsections] = {0};
int sck[nsections] = {0};
int ssgl[nsupersections] = {0} ;
int sspl[nsupersections] = {0} ;
int sscl[nsupersections] = {0} ;
int sumi[bits] = {0};

//Integer array of inputs in binary form
//int* bin1=NULL;
//int* bin2=NULL;
int bin1[ bits ];
int bin2[ bits ];

//Character array of inputs in hex form
char* hex1=NULL;
char* hex2=NULL;

int ConvertHexToBinary( const char * hex , int * bin ) ;

int main( int argc , char ** agrv ) {
    /*
    char buffer[ digits ] ;
    int n ;
    n = 0 ;
    while( scanf( "%[^\n]%*c" , buffer ) == 1 ) {
        n++ ;
        printf( "Line%d: %s\n" , n , buffer ) ;
    }
    */

    /***************************************************
     * Read hex numbers into buffers
     **************************************************/
    char buffer1[ digits ] ;
    char buffer2[ digits ] ;
    if( scanf( "%s %s" , buffer1 , buffer2 ) != 2 ) {
        printf( "Number of input hex is not 2...\n") ;
        return EXIT_FAILURE ;
    }
    printf( "Buffer1: %s\n" , buffer1 ) ;
    printf( "Buffer2: %s\n" , buffer2 ) ;

    int bin[ 12 ] ;
    ConvertHexToBinary( "FA0" , bin ) ;
    int j ;
    for( j = 0 ; j < 12 ; j++ ) {
        printf( "bin[%d] = %d\n" , j , bin[j] ) ;
    }

    return EXIT_SUCCESS ;
}

int ConvertHexToBinary( const char * hex , int * bin ) {
    int i ;
    int j ;
    int h_len ;
    int b_len ;
    h_len = strlen( hex ) ;
    b_len = h_len * 4 ;
    j = b_len - 1 ;

    for( i = 0 ; i < h_len ; i++ ) {
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
