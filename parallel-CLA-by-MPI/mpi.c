/*
 * C program to parallel the sum of two 1,048,576-bit numbers by uing MPI (message passing interface)
 * Compute speedup of parallel MPI CLA relative to serial MPI CLA 
 * Compute speedup of parallel MPI CLA relative to serial RCA(ripple carry adder)
 * Jinghua Feng, fengj3@rpi.edu
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <mpi.h>

// EXAMPLE DATA STRUCTURE DESIGN AND LAYOUT FOR CLA
#define MY_INPUT_SIZE 262144
#define BLOCK_SIZE 32 
#define BITS MY_INPUT_SIZE*4

// Add 1 to array size because strings must be null terminated
char HEX_INPUT_A[ MY_INPUT_SIZE + 1 ] = {0};
char HEX_INPUT_B[ MY_INPUT_SIZE + 1 ] = {0};

//Integer array of inputs in binary form
int BIN1[ BITS ];
int BIN2[ BITS ];

//Character array for outputs in hex form
char HEX_OUTPUT[ MY_INPUT_SIZE + 1 ] ;

int ReadInput( const char * fname ) ; 
int WriteOutput( const char * fname ) ;
int CarryLookaheadAdder( const int * bin1 , const int * bin2 , const int bits ) ;
int RippleCarryAdder() ;
int ConvertHexToBinary( const char * hex , int * bin ) ;
int ComputeGiPi( const int * bin1 , const int * bin2 , const int bits , int * gi , int * pi ) ;
int ComputeGgjGpj( const int * gi , const int * pi , const int ngroups , int * ggj , int * gpj ) ;
int ComputeSgkSpk( const int * ggj , const int * gpj , const int nsections , int * sgk , int * spk ) ;
int ComputeSsglSspl( const int * sgk , const int * spk , const int nsupersections , int * ssgl , int * sspl ) ;
int ComputeSscl( const int * ssgl , const int * sspl , int * sscl ) ;
int ComputeSck( const int * sgk , const int * spk , const * sscl , int * sck ) ;
int ComputeGcj( const int * ggj , const int * gpj , const int * sck , int * gcj ) ;
int ComputeCi( const int * gi , const int * pi , const int * gcj , int * ci ) ;
int ComputeSumi( const int * bin1 , const int * bin2 , const int * bits , int * sumi ) ;
int ConvertBinaryToHex() ;
int PrintArray( const int * arr  , const int arr_size , const char * arr_name ) ;

int main( int argc , char ** argv ) {
    /*
    int my_mpi_size = -1;
    int my_mpi_rank = -1;
    MPI_Init( &argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &my_mpi_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_mpi_rank);
    */

    // ./a.out input.txt output.txt
    if( argc != 3 )
    {
        printf("Not sufficient arguments, only %d found \n", argc) ;
        exit( EXIT_FAILURE ) ;
    }

    ReadInput( argv[ 1 ] ) ;

     // Convert hex to binary and revert binary 
    ConvertHexToBinary( HEX_INPUT_A , bin1 ) ;
    ConvertHexToBinary( HEX_INPUT_B , bin2 ) ;
    // PrintArray( bin1 , BITS , "bin1" ) ; 
    // PrintArray( bin2 , BITS , "bin2" ) ; 

    // Carry-Lookahead Adder Algorithm 
    CarryLookaheadAdder( BIN1 , BIN2 , BITS ) ; 

    /* 
     * Ripper Carry Adder Algorithm * Uncomment below command line if like to check serial result of sumi and addition 
     */
    // RippleCarryAdder() ; 

    // Convert binary sumi to hex form 
    ConvertBinaryToHex() ; 

    WriteOutput( argv[ 2 ] ) ;

    //MPI_Finalize();
    return EXIT_SUCCESS ;
}

int ReadInput( const char * fname ) { 

    FILE * file ;
    if( ( file = fopen( fname , "r" ) ) == NULL ) {
        printf( "Failed to open input data file: %s\n" , fname ) ;
        exit( EXIT_FAILURE ) ;
    }
    fscanf( file , "%s %s" , HEX_INPUT_A , HEX_INPUT_B ) ;

    printf( "hex1:\n%s\n\n" , HEX_INPUT_A ) ;
    printf( "hex2:\n%s\n\n" , HEX_INPUT_B ) ;
    fclose( file ) ;

    return EXIT_SUCCESS ;
}

int WriteOutput( const char * fname ) { 

    FILE * file ;
    if( ( file = fopen( fname , "w" ) ) == NULL ) {
        printf( "Failed to open output data file: %s\n" , fname ) ;
        exit( EXIT_FAILURE ) ;
    }
    fprintf( file , "%s\n" , HEX_OUTPUT ) ;
    fclose( file ) ;

    return EXIT_SUCCESS ;
}

/*****************************************
 * CLA algorithm
 ****************************************/
int CarryLookaheadAdder( const int * bin1 , const int * bin2 , const int bits ) {
    //Do not touch these defines
    //#define digits (input_size+1)
    int ngroups = bits / BLOCK_SIZE ;
    int nsections = ngroups / BLOCK_SIZE ;
    int nsupersections = nsections / BLOCK_SIZE ;

    int ci[ bits ] = {0};
    int gi[ bits ] = {0};
    int pi[ bits ] = {0};

    int ggj[ngroups] = {0};
    int gpj[ngroups] = {0};
    int gcj[ngroups] = {0};

    int sgk[nsections] = {0};
    int spk[nsections] = {0};
    int sck[nsections] = {0};

    int ssgl[nsupersections] = {0} ;
    int sspl[nsupersections] = {0} ;
    int sscl[nsupersections] = {0} ;

    int sumi[BITS] = {0};


    ComputeGiPi( bin1 , bin2 , bits , gi , pi ) ;
    ComputeGgjGpj( gi , pi , ngroups , ggj , gpj ) ;
    ComputeSgkSpk( ggj , gpj , nsections , spk , spk ) ;
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
    for( i = 0 ; i < BITS ; i++ ){
        if( i == 0 ) ci[ i ] = gi[ i ] || ( pi[ i ] && 0 ) ;
        else ci[ i ] = gi[ i ] || ( pi[ i ] && ci[ i -1 ] ) ;
    }

    // Compute sumi
    for( i = 0 ; i < BITS ; i++ ){
        if( i == 0 ) sumi[ i ] = bin1[ i ] ^ bin2[ i ] ^ 0 ;
        else sumi[ i ] = bin1[ i ] ^ bin2[ i ] ^ ci[ i - 1 ] ;
    }

    // PrintArray( ci , BITS , "ripple ci" ) ;
    // PrintArray( sumi , BITS , "ripple sumi" ) ;

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

int ConvertBinaryToHex() {

    /*
     * i: iter for sumi
     * j: iter for HEX_OUTPUT (final output) 
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

        if ( my_sum <= 9 ) HEX_OUTPUT[ j ] = my_sum  + '0' ;
        else 
        {
            switch( my_sum ) {
                case 10 :
                    HEX_OUTPUT[ j ] = 'A' ; break ;
                case 11 :
                    HEX_OUTPUT[ j ] = 'B' ; break ;
                case 12 :
                    HEX_OUTPUT[ j ] = 'C' ; break ;
                case 13 :
                    HEX_OUTPUT[ j ] = 'D' ; break ;
                case 14 :
                    HEX_OUTPUT[ j ] = 'E' ; break ;
                case 15 :
                    HEX_OUTPUT[ j ] = 'F' ; break ;
                default :
                    printf( "Invalid hex number...\n" ) ;
                    return EXIT_FAILURE ;
            }
        }
        
        j++ ;
    }

    // printf( "Addition:\n%s\n\n" , HEX_OUTPUT ) ;
    HEX_OUTPUT[ MY_INPUT_SIZE ] = '\0' ; 
    printf( "%s\n" , HEX_OUTPUT ) ;
    return EXIT_SUCCESS ;
}

/************************************************
 1. Calculate g_i and p_i for all BITS i
************************************************/
int ComputeGiPi( const int * bin1 , const int * bin2 , const int bits , int * gi , int * pi ) { 
    int i ;
    for( i = 0 ; i < bits ; i++ ) {
        gi[ i ] = bin1[ i ] && bin2[ i ] ;
        pi[ i ] = bin1[ i ] || bin2[ i ] ; 
        //gi[ i ] = bin1[ i ] * bin2[ i ] ;
        //pi[ i ] = bin1[ i ] + bin2[ i ] - gi[ i ] ;
    }
    //PrintArray( gi , bits , "gi" ) ;
    //PrintArray( pi , bits , "pi" ) ;
    return EXIT_SUCCESS ;
}

/************************************************
 2. Calculate gg_j and gp_j for all 512 groups j 
************************************************/
int ComputeGgjGpj( const int * gi , const int * pi , const int ngroups , int * ggj , int * gpj ){
    int j , i , ii , jj , tmp ; 
    for( j = 0 ; j < ngroups ; j++ ) {
        i = j * BLOCK_SIZE ;

        ggj[ j ] = gi[ i + BLOCK_SIZE - 1 ] ;
        gpj[ j ] = pi[ i + BLOCK_SIZE - 1 ] ;

        for( ii = BLOCK_SIZE - 1 ; ii > 0 ; ii-- ) {
            tmp = gi[ i + ii - 1 ] ;
            for( jj = BLOCK_SIZE - 1 ; jj >= ii ; jj-- )
                tmp = tmp && pi[ i + jj ] ; 
            ggj[ j ] = ggj[ j ] || tmp ;

            gpj[ j ] = gpj[ j ] && pi[ i + ii - 1 ] ;
        }
        /*
        ggj[ j ] = gi[ i + 7 ] || (
                   pi[ i + 7 ] && gi[ i + 6 ] ) || ( 
                   pi[ i + 7 ] && pi[ i + 6 ] && gi[ i + 5 ] ) || (
                   pi[ i + 7 ] && pi[ i + 6 ] && pi[ i + 5 ] && gi[ i + 4 ] ) || (
                   pi[ i + 7 ] && pi[ i + 6 ] && pi[ i + 5 ] && pi[ i + 4 ] && gi[ i + 3 ] ) || (
                   pi[ i + 7 ] && pi[ i + 6 ] && pi[ i + 5 ] && pi[ i + 4 ] && pi[ i + 3 ] && gi[ i + 2 ] ) || (
                   pi[ i + 7 ] && pi[ i + 6 ] && pi[ i + 5 ] && pi[ i + 4 ] && pi[ i + 3 ] && pi[ i + 2 ] && gi[ i + 1 ] ) || (
                   pi[ i + 7 ] && pi[ i + 6 ] && pi[ i + 5 ] && pi[ i + 4 ] && pi[ i + 3 ] && pi[ i + 2 ] && pi[ i + 1 ] && gi[ i ] ) ;
        gpj[ j ] = pi[ i + 7 ] && pi[ i + 6 ] && pi[ i + 5 ] && pi[ i + 4 ] && pi[ i + 3 ] && pi[ i + 2 ] && pi[ i + 1 ] && pi[ i ] ;
       */
    }

    //PrintArray( ggj , ngroups , "ggj" ) ;
    //PrintArray( gpj , ngroups , "gpj" ) ;
    return EXIT_SUCCESS ;
}

/************************************************
 3. Calculate sg_k and sp_k for all 64 groups k 
************************************************/
int ComputeSgkSpk( const int * ggj , const int * gpj , const int nsections , int * sgk , int * spk ){
    int k , j , ii , jj , tmp ; 
    for( k = 0 ; k < nsections ; k++ ) {
        j = k * BLOCK_SIZE ;

        sgk[ k ] = ggj[ j + BLOCK_SIZE - 1 ] ;
        spk[ k ] = gpj[ j + BLOCK_SIZE - 1 ] ;

        for( ii = BLOCK_SIZE - 1 ; ii > 0 ; ii-- ) {
            tmp = ggj[ j + ii - 1 ] ;
            for( jj = BLOCK_SIZE - 1 ; jj >= ii ; jj-- )
                tmp = tmp && gpj[ j + jj ] ; 
            sgk[ k ] = sgk[ k ] || tmp ;

            spk[ k ] = spk[ k ] && gpj[ j + ii - 1 ] ;
        }
        /*
        j = k * BLOCK_SIZE ;
        sgk[ k ] = ggj[ j + 7 ] || (
                   gpj[ j + 7 ] && ggj[ j + 6 ] ) || ( 
                   gpj[ j + 7 ] && gpj[ j + 6 ] && ggj[ j + 5 ] ) || (
                   gpj[ j + 7 ] && gpj[ j + 6 ] && gpj[ j + 5 ] && ggj[ j + 4 ] ) || (
                   gpj[ j + 7 ] && gpj[ j + 6 ] && gpj[ j + 5 ] && gpj[ j + 4 ] && ggj[ j + 3 ] ) || (
                   gpj[ j + 7 ] && gpj[ j + 6 ] && gpj[ j + 5 ] && gpj[ j + 4 ] && gpj[ j + 3 ] && ggj[ j + 2 ] ) || (
                   gpj[ j + 7 ] && gpj[ j + 6 ] && gpj[ j + 5 ] && gpj[ j + 4 ] && gpj[ j + 3 ] && gpj[ j + 2 ] && ggj[ j + 1 ] ) || (
                   gpj[ j + 7 ] && gpj[ j + 6 ] && gpj[ j + 5 ] && gpj[ j + 4 ] && gpj[ j + 3 ] && gpj[ j + 2 ] && gpj[ j + 1 ] && ggj[ j ] ) ;

        spk[ k ] = gpj[ j + 7 ] && gpj[ j + 6 ] && gpj[ j + 5 ] && gpj[ j + 4 ] && gpj[ j + 3 ] && gpj[ j + 2 ] && gpj[ j + 1 ] && gpj[ j ] ;
        */
    }

    //PrintArray( sgk , nsections , "sgk" ) ;
    //PrintArray( spk , nsections, "spk" ) ;

    return EXIT_SUCCESS ;
}

/******************************************************
 4. Calculate ssg_l and ssp_l for all BLOCK_SIZE super sections j 
******************************************************/
int ComputeSsglSspl( const int * sgk , const int * spk , const int nsupersections , int * ssgl , int * sspl ){
    int l , k , ii , jj , tmp ;
    for( l = 0 ; l < nsupersections ; l++ ) {
        k = l * BLOCK_SIZE ;

        ssgl[ l ] = sgk[ k + BLOCK_SIZE - 1 ] ;
        sspl[ l ] = spk[ k + BLOCK_SIZE - 1 ] ;

        for( ii = BLOCK_SIZE - 1 ; ii > 0 ; ii-- ) {
            tmp = sgk[ k + ii - 1 ] ;
            for( jj = BLOCK_SIZE - 1 ; jj >= ii ; jj-- )
                tmp = tmp && spk[ k + jj ] ; 
            ssgl[ l ] = ssgl[ l ] || tmp ;

            sspl[ l ] = sspl[ l ] && spk[ k + ii - 1 ] ;
        }
        /*
        ssgl[ l ] = sgk[ k + 7 ] || (
                    spk[ k + 7 ] && sgk[ k + 6 ] ) || ( 
                    spk[ k + 7 ] && spk[ k + 6 ] && sgk[ k + 5 ] ) || (
                    spk[ k + 7 ] && spk[ k + 6 ] && spk[ k + 5 ] && sgk[ k + 4 ] ) || (
                    spk[ k + 7 ] && spk[ k + 6 ] && spk[ k + 5 ] && spk[ k + 4 ] && sgk[ k + 3 ] ) || (
                    spk[ k + 7 ] && spk[ k + 6 ] && spk[ k + 5 ] && spk[ k + 4 ] && spk[ k + 3 ] && sgk[ k + 2 ] ) || (
                    spk[ k + 7 ] && spk[ k + 6 ] && spk[ k + 5 ] && spk[ k + 4 ] && spk[ k + 3 ] && spk[ k + 2 ] && sgk[ k + 1 ] ) || (
                    spk[ k + 7 ] && spk[ k + 6 ] && spk[ k + 5 ] && spk[ k + 4 ] && spk[ k + 3 ] && spk[ k + 2 ] && spk[ k + 1 ] && sgk[ k ] ) ;

        sspl[ l ] = spk[ k + 7 ] && spk[ k + 6 ] && spk[ k + 5 ] && spk[ k + 4 ] && spk[ k + 3 ] && spk[ k + 2 ] && spk[ k + 1 ] && spk[ k ] ;
        */
    }

    // PrintArray( ssgl , nsupersections , "ssgl" ) ;
    // PrintArray( sspl , nsupersections, "sspl" ) ;

    return EXIT_SUCCESS ;
}

/******************************************************************************
 5. Calculate ssc_l using ssg_l and ssp_l for all l super sections and 0 for ssc−1 
******************************************************************************/
int ComputeSscl( const int * ssgl , const int * sspl , int * sscl ){
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
   l = k div BLOCK_SIZE as super sectional carry-in for all sections k.
******************************************************************************/
int ComputeSck( const int * sgk , const int * spk , const * sscl , int * sck ) {
    int k ;
    for( k = 0 ; k < nsections ; k++ ) {
        if( k == 0 ) 
            sck[ k ] = sgk[ k ] || ( spk[ k ] && 0 ) ;
        else if( k % BLOCK_SIZE == 0 )
            sck[ k ] = sgk[ k ] || ( spk[ k ] && sscl[ k/BLOCK_SIZE - 1 ] ) ;
        else 
            sck[ k ] = sgk[ k ] || ( spk[ k ] && sck[ k - 1 ] ) ;
    }
    // PrintArray( sck , nsections , "sck" ) ;

    return EXIT_SUCCESS ;
}

/******************************************************************************
 7. Calculate gcj using ggj, gpj and correct sck, 
    k = j div BLOCK_SIZE as sectional carry-in for all groups j
******************************************************************************/
int ComputeGcj( const int * ggj , const int * gpj , const int * sck , int * gcj ) {
    int j ;
    for( j = 0 ; j < ngroups ; j++ ) {
        if( j == 0 ) 
            gcj[ j ] = ggj[ j ] || ( gpj[ j ] && 0 ) ;
        else if( j % BLOCK_SIZE == 0 ) 
            gcj[ j ] = ggj[ j ] || ( gpj[ j ] && sck[ j/BLOCK_SIZE - 1 ] ) ;
        else 
            gcj[ j ] = ggj[ j ] || ( gpj[ j ] && gcj[ j - 1 ] ) ;
    }

    // PrintArray( gcj , ngroups , "gcj" ) ;

    return EXIT_SUCCESS ;
}

/******************************************************************************
 8.Calculate ci using gi, pi and correct gcj, 
   j = i div BLOCK_SIZE as group carry-in for all bits i
******************************************************************************/
int ComputeCi( const int * gi , const int * pi , const int * gcj , int * ci ) {
    int i ;
    for( i = 0 ; i < BITS ; i++ ) {
        if( i == 0 ) 
            ci[ i ] = gi[ i ] || ( pi[ i ] && 0 ) ;
        else if( i % BLOCK_SIZE == 0 ) 
            ci[ i ] = gi[ i ] || ( pi[ i ] && gcj[ i/BLOCK_SIZE -1 ] ) ;
        else 
            ci[ i ] = gi[ i ] || ( pi[ i ] && ci[ i - 1 ] ) ;
    }

    // PrintArray( ci , BITS , "ci" ) ;

    return EXIT_SUCCESS ;
}

/******************************************************************************
 9. Calculate sumi using a_i xor b_i xor c_i−1 for all i 
******************************************************************************/
int ComputeSumi( const int * bin1 , const int * bin2 , const int * bits , int * sumi ) {
    int i ;
    for( i = 0 ; i < bits ; i++ ){
        if( i == 0 ) sumi[ i ] = bin1[ i ] ^ bin2[ i ] ^ 0 ;
        else sumi[ i ] = bin1[ i ] ^ bin2[ i ] ^ ci[ i - 1 ] ;
    }

    // PrintArray( sumi , BITS , "sumi" ) ;

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
