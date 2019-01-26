/*
 * C program to parallel the sum of two 4096-bit numbers
 * Simulate a 4,096 bit Carry lookahead adder (CLA) by using 8-bit blocks
 * Use a 4,096 bit serial Ripple-carry adder to check the correctness of CLA
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

//Character array for outputs in hex form
char opt[ digits ] ;

int ReadInput() ; 
int CarryLookaheadAdder() ;
int RippleCarryAdder() ;
int ConvertHexToBinary( const char * hex , int * bin ) ;
int ComputeGiPi() ;
int ComputeGgjGpj() ;
int ComputeSgkSpk() ;
int ComputeSsglSspl() ;
int ComputeSscl() ;
int ComputeSck() ;
int ComputeGcj() ;
int ComputeCi() ;
int ComputeSumi() ;
int ConvertBinaryToHex() ; 
int PrintArray( const int * arr  , const int arr_size , const char * arr_name ) ;

int main( int argc , char ** agrv ) {
    
    // Read numbers from redirected file to array hex1 and hex2  
    ReadInput() ;

    // Convert hex to binary and revert binary 
    ConvertHexToBinary( hex1 , bin1 ) ;
    ConvertHexToBinary( hex2 , bin2 ) ;
    // PrintArray( bin1 , bits , "bin1" ) ; 
    // PrintArray( bin2 , bits , "bin2" ) ; 

    // Carry-Lookahead Adder Algorithm 
    CarryLookaheadAdder() ;

    /*
     * Ripper Carry Adder Algorithm
     * Uncomment below command line if like to check serial result of sumi and addition 
    */
    // RippleCarryAdder() ; 

    // Convert binary sumi to hex form 
    ConvertBinaryToHex() ; 

    return EXIT_SUCCESS ;
}

int ReadInput() { 

    // Add 0 to the begining of hex array
    hex1[ 0 ] = '0' ;
    hex2[ 0 ] = '0' ;

    // Read numbers from redirected file to array hex1 and hex2  
    if( scanf( "%s %s" , hex1 + 1 , hex2 + 1 ) != 2 ) {
        printf( "Number of input hex is not 2...\n") ;
        return EXIT_FAILURE ;
    }

    // printf( "hex1:\n%s\n\n" , hex1 ) ;
    // printf( "hex2:\n%s\n\n" , hex2 ) ;

    return EXIT_SUCCESS ;
}

/*****************************************
 * CLA algorithm
 ****************************************/
int CarryLookaheadAdder() {
    ComputeGiPi() ;
    ComputeGgjGpj() ;
    ComputeSgkSpk() ;
    ComputeSsglSspl() ;
    ComputeSscl() ;
    ComputeSck() ;
    ComputeGcj() ;
    ComputeCi() ;
    ComputeSumi() ;
    return EXIT_SUCCESS ;
}

/*****************************************
 * Serial ripple alorithm 
 ****************************************/
int RippleCarryAdder() {
    // Get carry-in array ci
    int i ;
    for( i = 0 ; i < bits ; i++ ){
        if( i == 0 ) ci[ i ] = gi[ i ] || ( pi[ i ] && 0 ) ;
        else ci[ i ] = gi[ i ] || ( pi[ i ] && ci[ i -1 ] ) ;
    }

    // Compute sumi
    for( i = 0 ; i < bits ; i++ ){
        if( i == 0 ) sumi[ i ] = bin1[ i ] ^ bin2[ i ] ^ 0 ;
        else sumi[ i ] = bin1[ i ] ^ bin2[ i ] ^ ci[ i - 1 ] ;
    }

    // PrintArray( ci , bits , "ripple ci" ) ;
    // PrintArray( sumi , bits , "ripple sumi" ) ;

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

int ConvertBinaryToHex() {

    /*
     * i: iter for sumi
     * j: iter for opt (final output) 
     * my_sum: sum of every four bits 
     */
    int i , j , my_sum ;
    i = bits - 1 ;
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

        if ( my_sum <= 9 ) opt[ j ] = my_sum  + '0' ;
        else 
        {
            switch( my_sum ) {
                case 10 :
                    opt[ j ] = 'A' ; break ;
                case 11 :
                    opt[ j ] = 'B' ; break ;
                case 12 :
                    opt[ j ] = 'C' ; break ;
                case 13 :
                    opt[ j ] = 'D' ; break ;
                case 14 :
                    opt[ j ] = 'E' ; break ;
                case 15 :
                    opt[ j ] = 'F' ; break ;
                default :
                    printf( "Invalid hex number...\n" ) ;
                    return EXIT_FAILURE ;
            }
        }
        
        j++ ;
    }

    // printf( "Addition:\n%s\n\n" , opt ) ;
    printf( "%s\n" , opt ) ;
    return EXIT_SUCCESS ;
}

/************************************************
 1. Calculate g_i and p_i for all 4100 bits i
************************************************/
int ComputeGiPi(){ 
    int i ;
    for( i = 0 ; i < bits ; i++ ) {
        gi[ i ] = bin1[ i ] && bin2[ i ] ;
        pi[ i ] = bin1[ i ] || bin2[ i ] ; 
        //gi[ i ] = bin1[ i ] * bin2[ i ] ;
        //pi[ i ] = bin1[ i ] + bin2[ i ] - gi[ i ] ;
    }
    // PrintArray( gi , bits , "gi" ) ;
    // PrintArray( pi , bits , "pi" ) ;
    return EXIT_SUCCESS ;
}

/************************************************
 2. Calculate gg_j and gp_j for all 512 groups j 
************************************************/
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

    // PrintArray( ggj , ngroups , "ggj" ) ;
    // PrintArray( gpj , ngroups , "gpj" ) ;
    return EXIT_SUCCESS ;
}

/************************************************
 3. Calculate sg_k and sp_k for all 64 groups k 
************************************************/
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

    // PrintArray( sgk , nsections , "sgk" ) ;
    // PrintArray( spk , nsections, "spk" ) ;

    return EXIT_SUCCESS ;
}

/******************************************************
 4. Calculate ssg_l and ssp_l for all 8 super sections j 
******************************************************/
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

    // PrintArray( ssgl , nsupersections , "ssgl" ) ;
    // PrintArray( sspl , nsupersections, "sspl" ) ;

    return EXIT_SUCCESS ;
}

/******************************************************************************
 5. Calculate ssc_l using ssg_l and ssp_l for all l super sections and 0 for ssc−1 
******************************************************************************/
int ComputeSscl(){
    int l ;
    for( l = 0 ; l < nsupersections ; l++ ) {
        if( l == 0 ) 
            sscl[ l ] = ssgl[ l ] || ( sspl[ l ] && 0 ) ;
        else 
            sscl[ l ] = ssgl[ l ] || ( sspl[ l ] && sscl[ l - 1 ] ) ;
    }
    // PrintArray( sscl , nsupersections , "sscl" ) ;

    return EXIT_SUCCESS ;
}


/******************************************************************************
 6.Calculate sck using sgk and spk and correct sscl,
   l = k div 8 as super sectional carry-in for all sections k.
******************************************************************************/
int ComputeSck(){
    int k ;
    for( k = 0 ; k < nsections ; k++ ) {
        if( k == 0 ) 
            sck[ k ] = sgk[ k ] || ( spk[ k ] && 0 ) ;
        else if( k % block_size == 0 )
            sck[ k ] = sgk[ k ] || ( spk[ k ] && sscl[ k/block_size - 1 ] ) ;
        else 
            sck[ k ] = sgk[ k ] || ( spk[ k ] && sck[ k - 1 ] ) ;
    }
    // PrintArray( sck , nsections , "sck" ) ;

    return EXIT_SUCCESS ;
}

/******************************************************************************
 7. Calculate gcj using ggj, gpj and correct sck, 
    k = j div 8 as sectional carry-in for all groups j
******************************************************************************/
int ComputeGcj(){
    int j ;
    for( j = 0 ; j < ngroups ; j++ ) {
        if( j == 0 ) 
            gcj[ j ] = ggj[ j ] || ( gpj[ j ] && 0 ) ;
        else if( j % block_size == 0 ) 
            gcj[ j ] = ggj[ j ] || ( gpj[ j ] && sck[ j/block_size - 1 ] ) ;
        else 
            gcj[ j ] = ggj[ j ] || ( gpj[ j ] && gcj[ j - 1 ] ) ;
    }

    // PrintArray( gcj , ngroups , "gcj" ) ;

    return EXIT_SUCCESS ;
}

/******************************************************************************
 8.Calculate ci using gi, pi and correct gcj, 
   j = i div 8 as group carry-in for all bits i
******************************************************************************/
int ComputeCi(){
    int i ;
    for( i = 0 ; i < bits ; i++ ) {
        if( i == 0 ) 
            ci[ i ] = gi[ i ] || ( pi[ i ] && 0 ) ;
        else if( i % block_size == 0 ) 
            ci[ i ] = gi[ i ] || ( pi[ i ] && gcj[ i/block_size -1 ] ) ;
        else 
            ci[ i ] = gi[ i ] || ( pi[ i ] && ci[ i - 1 ] ) ;
    }

    // PrintArray( ci , bits , "ci" ) ;

    return EXIT_SUCCESS ;
}

/******************************************************************************
 9. Calculate sumi using a_i xor b_i xor c_i−1 for all i 
******************************************************************************/
int ComputeSumi() {
    int i ;
    for( i = 0 ; i < bits ; i++ ){
        if( i == 0 ) sumi[ i ] = bin1[ i ] ^ bin2[ i ] ^ 0 ;
        else sumi[ i ] = bin1[ i ] ^ bin2[ i ] ^ ci[ i - 1 ] ;
    }

    // PrintArray( sumi , bits , "sumi" ) ;

    return EXIT_SUCCESS ;
}


/****************************************
 Print out array of int
****************************************/
int PrintArray( const int * arr  , const int arr_size , const char * arr_name ) {
    printf( "%s:\n" , arr_name ) ;
    int i ;
    for( i = 0 ; i < arr_size ; i++ ) {
        printf( "%d" , arr[i] ) ;
    }
    printf( "\n\n" ) ;

    return EXIT_SUCCESS ;
}
