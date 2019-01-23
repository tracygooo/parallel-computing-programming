/*
 * C program for a carry-lookahead adder
 * Simulate a 4,096 bit Carry lookahead adder by using 8-bit blocks
 * Jinghua Feng, fengj3@rpi.edu
 */
#include <stdio.h>
#include <stdlib.h>

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
int* bin1=NULL;
int* bin2=NULL;

//Character array of inputs in hex form
char* hex1=NULL;
char* hex2=NULL;

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

    char buffer1[ digits ] ;
    char buffer2[ digits ] ;
    if( scanf( "%s %s" , buffer1 , buffer2 ) != 2 ) {
        printf( "Number of input hex is not 2...\n") ;
        return EXIT_FAILURE ;
    }

    printf( "Buffer1: %s\n" , buffer1 ) ;
    printf( "Buffer2: %s\n" , buffer2 ) ;

    return 0 ;
}
