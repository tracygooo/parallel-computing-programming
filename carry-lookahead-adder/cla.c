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
//char* hex1=NULL;
//char* hex2=NULL;
char hex1[ digits ] ;
char hex2[ digits ] ;

int ConvertHexToBinary( const char * hex , int * bin ) ;
int ComputeGiPi() ;
int ComputeGgjGpj() ;
int ComputeSgkSpk() ;
int ComputeSsglSspl() ;
int ComputeSscl() ;
int ComputeSck() ;
int ComputeGcj() ;
int ComputeCi() ;
int PrintArray( const int * arr  , const int arr_size , const char * arr_name ) ;

int main( int argc , char ** agrv ) {
    
    /***********************************************************
     * Add 0 to the begining of hex array
     * Read numbers from redirected file to array hex1 and hex2  
     ***********************************************************/
    hex1[ 0 ] = '0' ;
    hex2[ 0 ] = '0' ;
    if( scanf( "%s %s" , hex1 + 1 , hex2 + 1 ) != 2 ) {
        printf( "Number of input hex is not 2...\n") ;
        return EXIT_FAILURE ;
    }
    printf( "hex1: %s\n" , hex1 ) ;
    printf( "hex2: %s\n" , hex2 ) ;

    /***************************************************
     * Convert hex to binary
     * Revert binary 
     **************************************************/
    ConvertHexToBinary( hex1 , bin1 ) ;
    ConvertHexToBinary( hex2 , bin2 ) ;
    int j ;
    printf( "bin1:" ) ;
    for( j = 0 ; j < bits ; j++ ) {
        printf( "%d" , bin1[j] ) ;
    }

    /***************************************************
     * Carry-Lookahead Adder Algorithm 
     **************************************************/
    ComputeGiPi() ;
    ComputeGgjGpj() ;
    ComputeSgkSpk() ;
    ComputeSsglSspl() ;
    ComputeSscl() ;
    ComputeSck() ;
    ComputeGcj() ;
    ComputeCi() ;

    /*
    printf( "ngroups: %d\n" , ngroups ) ;
    printf( "nsections: %d\n" , nsections ) ; 
    printf( "nsupersections: %d\n" , nsupersections ) ;
    */

    return EXIT_SUCCESS ;
}

int ConvertHexToBinary( const char * hex , int * bin ) {
    int i ;
    int j ;
    j = bits - 1 ;

    for( i = 0 ; i < digits ; i++ ) {
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

int ComputeGiPi(){ 
    int i ;
    for( i = 0 ; i < bits ; i++ ) {
        //gi[ i ] = bin1[ i ] * bin2[ i ] ;
        //pi[ i ] = bin1[ i ] + bin2[ i ] - gi[ i ] ;
        gi[ i ] = bin1[ i ] && bin2[ i ] ;
        pi[ i ] = bin1[ i ] || bin2[ i ] ; 
    }
    PrintArray( gi , bits , "gi" ) ;
    PrintArray( pi , bits , "pi" ) ;
    return EXIT_SUCCESS ;
}

int ComputeGgjGpj(){
    int j ;
    int i ;
    for( j = 0 ; j < ngroups ; j++ ) {
        i = j * 8 ;
        ggj[ j ] = gi[ i + 7 ] || (
                   pi[ i + 7 ] && gi[ i + 6 ] ) || ( 
                   pi[ i + 7 ] && pi[ i + 6 ] && gi[ i + 5 ] ) || (
                   pi[ i + 7 ] && pi[ i + 6 ] && pi[ i + 5 ] && gi[ i + 4 ] ) || (
                   pi[ i + 7 ] && pi[ i + 6 ] && pi[ i + 5 ] && pi[ i + 4 ] && gi[ i + 3 ] ) || (
                   pi[ i + 7 ] && pi[ i + 6 ] && pi[ i + 5 ] && pi[ i + 4 ] && pi[ i + 3 ] && gi[ i + 2 ] ) || (
                   pi[ i + 7 ] && pi[ i + 6 ] && pi[ i + 5 ] && pi[ i + 4 ] && pi[ i + 3 ] && pi[ i + 2 ] && gi[ i + 1 ] ) || (
                   pi[ i + 7 ] && pi[ i + 6 ] && pi[ i + 5 ] && pi[ i + 4 ] && pi[ i + 3 ] && pi[ i + 2 ] && pi[ i + 1 ] && gi[ i ] ) ;

        gpj[ j ] = pi[ i + 7 ] && pi[ i + 6 ] && pi[ i + 5 ] && pi[ i + 4 ] && pi[ i + 3 ] && pi[ i + 2 ] && pi[ i + 1 ] && pi[ i ] ;
    }

    PrintArray( ggj , ngroups , "ggj" ) ;
    PrintArray( gpj , ngroups , "gpj" ) ;

    return EXIT_SUCCESS ;
}

int ComputeSgkSpk(){
    int k ;
    int j ;
    for( k = 0 ; k < nsections ; k++ ) {
        j = k * 8 ;
        sgk[ k ] = ggj[ j + 7 ] || (
                   gpj[ j + 7 ] && ggj[ j + 6 ] ) || ( 
                   gpj[ j + 7 ] && gpj[ j + 6 ] && ggj[ j + 5 ] ) || (
                   gpj[ j + 7 ] && gpj[ j + 6 ] && gpj[ j + 5 ] && ggj[ j + 4 ] ) || (
                   gpj[ j + 7 ] && gpj[ j + 6 ] && gpj[ j + 5 ] && gpj[ j + 4 ] && ggj[ j + 3 ] ) || (
                   gpj[ j + 7 ] && gpj[ j + 6 ] && gpj[ j + 5 ] && gpj[ j + 4 ] && gpj[ j + 3 ] && ggj[ j + 2 ] ) || (
                   gpj[ j + 7 ] && gpj[ j + 6 ] && gpj[ j + 5 ] && gpj[ j + 4 ] && gpj[ j + 3 ] && gpj[ j + 2 ] && ggj[ j + 1 ] ) || (
                   gpj[ j + 7 ] && gpj[ j + 6 ] && gpj[ j + 5 ] && gpj[ j + 4 ] && gpj[ j + 3 ] && gpj[ j + 2 ] && gpj[ j + 1 ] && ggj[ j ] ) ;

        spk[ k ] = gpj[ j + 7 ] && gpj[ j + 6 ] && gpj[ j + 5 ] && gpj[ j + 4 ] && gpj[ j + 3 ] && gpj[ j + 2 ] && gpj[ j + 1 ] && gpj[ j ] ;
    }

    PrintArray( sgk , nsections , "sgk" ) ;
    PrintArray( spk , nsections, "spk" ) ;

    return EXIT_SUCCESS ;
}

int ComputeSsglSspl(){
    int l ;
    int k ;
    for( l = 0 ; l < nsupersections ; l++ ) {
        k = l * 8 ;
        ssgl[ l ] = sgk[ k + 7 ] || (
                   spk[ k + 7 ] && sgk[ k + 6 ] ) || ( 
                   spk[ k + 7 ] && spk[ k + 6 ] && sgk[ k + 5 ] ) || (
                   spk[ k + 7 ] && spk[ k + 6 ] && spk[ k + 5 ] && sgk[ k + 4 ] ) || (
                   spk[ k + 7 ] && spk[ k + 6 ] && spk[ k + 5 ] && spk[ k + 4 ] && sgk[ k + 3 ] ) || (
                   spk[ k + 7 ] && spk[ k + 6 ] && spk[ k + 5 ] && spk[ k + 4 ] && spk[ k + 3 ] && sgk[ k + 2 ] ) || (
                   spk[ k + 7 ] && spk[ k + 6 ] && spk[ k + 5 ] && spk[ k + 4 ] && spk[ k + 3 ] && spk[ k + 2 ] && sgk[ k + 1 ] ) || (
                   spk[ k + 7 ] && spk[ k + 6 ] && spk[ k + 5 ] && spk[ k + 4 ] && spk[ k + 3 ] && spk[ k + 2 ] && spk[ k + 1 ] && sgk[ k ] ) ;

        sspl[ l ] = spk[ k + 7 ] && spk[ k + 6 ] && spk[ k + 5 ] && spk[ k + 4 ] && spk[ k + 3 ] && spk[ k + 2 ] && spk[ k + 1 ] && spk[ k ] ;
    }

    PrintArray( ssgl , nsupersections , "ssgl" ) ;
    PrintArray( sspl , nsupersections, "sspl" ) ;

    return EXIT_SUCCESS ;
}

int ComputeSscl(){
    int l ;
    for( l = 0 ; l < nsupersections ; l++ ) {
        if( l == 0 ) sscl[ l ] = ssgl[ l ] || ( sspl[ l ] && 0 ) ;
        else sscl[ l ] = ssgl[ l ] || ( sspl[ l ] && sscl[ l - 1 ] ) ;
    }
    PrintArray( sscl , nsupersections , "sscl" ) ;

    return EXIT_SUCCESS ;
}

int ComputeSck(){
    int k ;
    for( k = 0 ; k < nsections ; k++ ) {
        if( k % block_size == 0 ) sck[ k ] = sgk[ k ] || ( spk[ k ] && sscl[ k/block_size ] ) ;
        else sck[ k ] = sgk[ k ] || ( spk[ k ] && sck[ k - 1 ] ) ;
    }
    PrintArray( sck , nsections , "sck" ) ;

    return EXIT_SUCCESS ;
}

int ComputeGcj(){
    int j ;
    for( j = 0 ; j < ngroups ; j++ ) {
        if( j % block_size == 0 ) 
            gcj[ j ] = ggj[ j ] || ( gpj[ j ] && sck[ j/block_size ] ) ;
        else 
            gcj[ j ] = ggj[ j ] || ( gpj[ j ] && gcj[ j - 1 ] ) ;
    }

    PrintArray( gcj , ngroups , "gcj" ) ;

    return EXIT_SUCCESS ;
}

int ComputeCi(){
    int i ;
    for( i = 0 ; i < ngroups ; i++ ) {
        if( i % block_size == 0 ) 
            ci[ i ] = gi[ i ] || ( pi[ i ] && gcj[ i/block_size ] ) ;
        else 
            ci[ i ] = gi[ i ] || ( pi[ i ] && ci[ i - 1 ] ) ;
    }

    PrintArray( ci , bits , "ci" ) ;

    return EXIT_SUCCESS ;
}

int PrintArray( const int * arr  , const int arr_size , const char * arr_name ) {
    printf( "\n%s:\n" , arr_name ) ;
    int i ;
    for( i = 0 ; i < arr_size ; i++ ) {
        printf( "%d" , arr[i] ) ;
    }
    return EXIT_SUCCESS ;
}








