/*
 * C program to parallel the sum of two 1,048,576-bit numbers by uing MPI (message passing interface)
 * Compute speedup of parallel MPI CLA relative to serial MPI CLA 
 * Compute speedup of parallel MPI CLA relative to serial RCA(ripple carry adder)
 * Jinghua Feng, fengj3@rpi.edu
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

// EXAMPLE DATA STRUCTURE DESIGN AND LAYOUT FOR CLA
#define MY_INPUT_SIZE 262144
#define BITS MY_INPUT_SIZE * 4
#define BLOCK_SIZE 32 

int ReadInput( const char * fname  , char * hex_input_a , char * hex_input_b ) ;
int WriteOutput( const char * fname , const char * hex_output ) ;
int CarryLookaheadAdder( const int * bin1 , const int * bin2 , const int bits , 
                         const int mpi_size , const int rank , const int ss_carry_in , MPI_Request * request , int * sumi ) ;
int RippleCarryAdder( const int * gi , const int * pi , const int bits , int * sumi ) ;
int ConvertHexToBinary( const char * hex , int * bin ) ;
int ComputeGiPi( const int * bin1 , const int * bin2 , const int bits , int * gi , int * pi ) ;
int ComputeGgjGpj( const int * gi , const int * pi , const int ngroups , int * ggj , int * gpj ) ;
int ComputeSgkSpk( const int * ggj , const int * gpj , const int nsections , int * sgk , int * spk ) ;
int ComputeSsglSspl( const int * sgk , const int * spk , const int nsupersections , int * ssgl , int * sspl ) ;
int ComputeSscl( const int * ssgl , const int * sspl , const int nsupersections , 
                 const int mpi_size , const int rank , const int carry_in , MPI_Request * request , int * sscl );
int ComputeSck( const int * sgk , const int * spk , const int * sscl , const int nsections , int * sck ) ;
int ComputeGcj( const int * ggj , const int * gpj , const int * sck , const int ngroups , int * gcj ) ;
int ComputeCi( const int * gi , const int * pi , const int * gcj , const int bits , int * ci ) ;
int ComputeSumi( const int * bin1 , const int * bin2 , const int bits , const int * ci , int * sumi ) ;
int ConvertBinaryToHex( const int * sumi , const int bits , char * hex_output ) ;
int PrintArray( const int * arr  , const int arr_size , const char * arr_name ) ;

int main( int argc , char ** argv ) {

    int my_mpi_size = -1;
    int my_mpi_rank = -1;

    // Char array of inputs in hex form
    char * HEX_INPUT_A ;
    char * HEX_INPUT_B ;

    // Integer array of inputs in binary form
    int * BIN1 ;
    int * BIN2 ;
    int * SUMI ;
    
    int chunk_size ; 

    // Character array for outputs in hex form
    char * HEX_OUTPUT ;

    MPI_Init( &argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &my_mpi_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_mpi_rank);

    chunk_size = BITS / my_mpi_size ;
    int * local_bin1 = ( int * ) malloc( chunk_size * sizeof( int ) ) ;
    int * local_bin2 = ( int * ) malloc( chunk_size * sizeof( int ) ) ;
    int * local_sum = ( int * ) malloc( chunk_size * sizeof( int ) ) ;

    // ./a.out input.txt output.txt
    if( argc != 3 )
    {
        printf("Not sufficient arguments, only %d found \n", argc) ;
        exit( EXIT_FAILURE ) ;
    }

    if( 0 == my_mpi_rank ) {

        // Add 1 to array size because strings must be null terminated
        HEX_INPUT_A = ( char * ) malloc( MY_INPUT_SIZE + 1 ) ;
        HEX_INPUT_B = ( char * ) malloc( MY_INPUT_SIZE + 1 ) ;

        //Integer array of inputs in binary form
        BIN1 = ( int * ) malloc( BITS * sizeof( int ) );
        BIN2 = ( int * ) malloc( BITS * sizeof( int ) );
        SUMI = ( int * ) malloc( BITS * sizeof( int ) );

        //Character array for outputs in hex form
        HEX_OUTPUT = ( char * ) malloc( MY_INPUT_SIZE + 1 ) ;

        ReadInput( argv[ 1 ] , HEX_INPUT_A , HEX_INPUT_B ) ;

        // Convert hex to binary and revert binary 
        ConvertHexToBinary( HEX_INPUT_A , BIN1 ) ;
        ConvertHexToBinary( HEX_INPUT_B , BIN2 ) ;
        //PrintArray( BIN1 , BITS , "bin1" ) ; 
        //PrintArray( BIN2 , BITS , "bin2" ) ; 
        
        //MPI_Scatter( BIN1 + my_mpi_rank * chunk_size , chunk_size , MPI_INT, 
        //             local_bin1 , chunk_size , MPI_INT , 0 , MPI_COMM_WORLD ) ;

        MPI_Scatter( BIN1 , chunk_size , MPI_INT, local_bin1 , chunk_size , MPI_INT , 0 , MPI_COMM_WORLD ) ;
        MPI_Scatter( BIN2 , chunk_size , MPI_INT, local_bin2 , chunk_size , MPI_INT , 0 , MPI_COMM_WORLD ) ;

        PrintArray(  local_bin1 , chunk_size , "Master local_bin1" ) ; 
        PrintArray(  local_bin2 , chunk_size , "Master local_bin2" ) ; 
        // Convert binary sumi to hex form 
        ConvertBinaryToHex( SUMI , BITS  , HEX_OUTPUT ) ;
        WriteOutput( argv[ 2 ] , HEX_OUTPUT ) ;

        free( BIN1 ) ;
        free( BIN2 ) ;
        free( SUMI ) ;
        free( HEX_INPUT_A ) ;
        free( HEX_INPUT_B ) ;
        free( HEX_OUTPUT ) ;
    }
    else {
        MPI_Scatter( BIN1 , chunk_size , MPI_INT, local_bin1 , chunk_size , MPI_INT , 0 , MPI_COMM_WORLD ) ;
        MPI_Scatter( BIN2 , chunk_size , MPI_INT, local_bin2 , chunk_size , MPI_INT , 0 , MPI_COMM_WORLD ) ;

        printf( "Rank: %d\n" , my_mpi_rank ) ;
        PrintArray(  local_bin1 , chunk_size , "Worker local_bin1" ) ; 
        PrintArray(  local_bin2 , chunk_size , "Worker local_bin2" ) ; 

        int ss_carry_in ;
        MPI_Request irecv_request ;
        MPI_Irecv( & ss_carry_in , 1 , MPI_INT , my_mpi_rank - 1 , 0 , MPI_COMM_WORLD , & irecv_request ) ;

        // Carry-Lookahead Adder Algorithm 
        CarryLookaheadAdder( local_bin1 , local_bin2 , chunk_size , 
                             my_mpi_size , my_mpi_rank , ss_carry_in , & irecv_request , local_sum ) ;

        MPI_Gather( local_sum , chunk_size , MPI_INT ,
                    SUMI , chunk_size , MPI_INT , 0 , MPI_COMM_WORLD ) ;
        
        free( local_bin1 ) ;
        free( local_bin2 ) ;
        free( local_sum ) ;
    }

    // Ripper Carry Adder Algorithm * Uncomment below command line if like to check serial result of sumi and addition 
    // RippleCarryAdder( BIN1 , BIN2 , BITS , SUMI ) ; 

    MPI_Finalize();

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

/*****************************************
 * CLA algorithm
 ****************************************/
int CarryLookaheadAdder( const int * bin1 , const int * bin2 , const int bits , 
                         const int mpi_size , const int rank , const int ss_carry_in , MPI_Request * request , int * sumi ) {
    int ngroups = bits / BLOCK_SIZE ;
    int nsections = ngroups / BLOCK_SIZE ;
    int nsupersections = nsections / BLOCK_SIZE ;

    int * gi = ( int * ) malloc( bits * sizeof( int ) ) ;
    int * pi = ( int * ) malloc( bits * sizeof( int ) ) ;
    int * ci = ( int * ) malloc( bits * sizeof( int ) ) ;

    int * ggj = ( int * ) malloc( ngroups * sizeof( int ) ) ;
    int * gpj = ( int * ) malloc( ngroups * sizeof( int ) ) ;
    int * gcj = ( int * ) malloc( ngroups * sizeof( int ) ) ;

    int * sgk = ( int * ) malloc( nsections * sizeof( int ) ) ;
    int * spk = ( int * ) malloc( nsections * sizeof( int ) ) ;
    int * sck = ( int * ) malloc( nsections * sizeof( int ) ) ;

    int * ssgl = ( int * ) malloc( nsupersections * sizeof( int ) ) ;
    int * sspl = ( int * ) malloc( nsupersections * sizeof( int ) ) ;
    int * sscl = ( int * ) malloc( nsupersections * sizeof( int ) ) ;

    ComputeGiPi( bin1 , bin2 , bits , gi , pi ) ;
    ComputeGgjGpj( gi , pi , ngroups , ggj , gpj ) ;
    ComputeSgkSpk( ggj , gpj , nsections , sgk , spk ) ;
    ComputeSsglSspl( sgk , spk , nsupersections , ssgl , sspl ) ;
    ComputeSscl( ssgl , sspl , nsupersections , mpi_size , rank , ss_carry_in , request , sscl ) ;
    ComputeSck( sgk , spk , sscl , nsections , sck ) ;
    ComputeGcj( ggj , gpj , sck , ngroups , gcj ) ;
    ComputeCi( gi , pi , gcj , bits , ci ) ;
    ComputeSumi( bin1 , bin2 , bits , ci , sumi ) ;

    free( gi ) ;
    free( pi ) ;
    free( ci ) ;

    free( ggj ) ;
    free( gpj ) ;
    free( gcj ) ;

    free( sgk ) ;
    free( spk ) ;
    free( sck ) ;

    free( ssgl ) ;
    free( sspl ) ;
    free( sscl ) ;

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
    // PrintArray( gi , bits , "gi" ) ;
    // PrintArray( pi , bits , "pi" ) ;
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

    // PrintArray( ggj , ngroups , "ggj" ) ;
    // PrintArray( gpj , ngroups , "gpj" ) ;
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

    // PrintArray( sgk , nsections , "sgk" ) ;
    // PrintArray( spk , nsections, "spk" ) ;

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

    //PrintArray( ssgl , nsupersections , "ssgl" ) ;
    //PrintArray( sspl , nsupersections, "sspl" ) ;

    return EXIT_SUCCESS ;
}

/******************************************************************************
 5. Calculate ssc_l using ssg_l and ssp_l for all l super sections and 0 for ssc−1 
******************************************************************************/
int ComputeSscl( const int * ssgl , const int * sspl , const int nsupersections , 
                 const int mpi_size , const int rank , const int carry_in , MPI_Request * request , int * sscl ){
    int l ; 
    MPI_Status irecv_status ;
    for( l = 0 ; l < nsupersections ; l++ ) {
        if( l == 0 ) { 
            if( rank == 0 )
                sscl[ l ] = ssgl[ l ] || ( sspl[ l ] && 0 ) ;
            else {
                MPI_Wait( request , & irecv_status ) ;
                sscl[ l ] = ssgl[ l ] || ( sspl[ l ] && carry_in ) ;
            }
        }
        else 
            sscl[ l ] = ssgl[ l ] || ( sspl[ l ] && sscl[ l - 1 ] ) ;
    }
    // PrintArray( sscl , nsupersections , "sscl" ) ;

    /*================================================================================= 
     * For example, if mpi_size = 32, execute MPI_Isend when rank = 0, 1, ..., 30
     * When rank = 31, only execute MPI_Irecv 
     * Send the last integer of array sscl to next rank 
     *================================================================================= */
    if( rank <  mpi_size - 1 )
        MPI_Isend( sscl + nsupersections - 1 , 1 , MPI_INT , rank + 1 , 0 , MPI_COMM_WORLD , request ) ;

    return EXIT_SUCCESS ;
}


/******************************************************************************
 6.Calculate sck using sgk and spk and correct sscl,
   l = k div BLOCK_SIZE as super sectional carry-in for all sections k.
******************************************************************************/
int ComputeSck( const int * sgk , const int * spk , const int * sscl , const int nsections , int * sck ) {
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
int ComputeGcj( const int * ggj , const int * gpj , const int * sck , const int ngroups , int * gcj ) {
    int j ;
    for( j = 0 ; j < ngroups ; j++ ) {
        if( j == 0 ) 
            gcj[ j ] = ggj[ j ] || ( gpj[ j ] && 0 ) ;
        else if( j % BLOCK_SIZE == 0 ) 
            gcj[ j ] = ggj[ j ] || ( gpj[ j ] && sck[ j/BLOCK_SIZE - 1 ] ) ;
        else 
            gcj[ j ] = ggj[ j ] || ( gpj[ j ] && gcj[ j - 1 ] ) ;
    }

    //PrintArray( gcj , ngroups , "gcj" ) ;

    return EXIT_SUCCESS ;
}

/******************************************************************************
 8.Calculate ci using gi, pi and correct gcj, 
   j = i div BLOCK_SIZE as group carry-in for all bits i
******************************************************************************/
int ComputeCi( const int * gi , const int * pi , const int * gcj , const int bits , int * ci ) {
    int i ;
    for( i = 0 ; i < BITS ; i++ ) {
        if( i == 0 ) 
            ci[ i ] = gi[ i ] || ( pi[ i ] && 0 ) ;
        else if( i % BLOCK_SIZE == 0 ) 
            ci[ i ] = gi[ i ] || ( pi[ i ] && gcj[ i/BLOCK_SIZE -1 ] ) ;
        else 
            ci[ i ] = gi[ i ] || ( pi[ i ] && ci[ i - 1 ] ) ;
    }

    // PrintArray( ci , bits , "ci" ) ;

    return EXIT_SUCCESS ;
}

/******************************************************************************
 9. Calculate sumi using a_i xor b_i xor c_i−1 for all i 
******************************************************************************/
int ComputeSumi( const int * bin1 , const int * bin2 , const int bits , const int * ci , int * sumi ) {
    int i ;
    for( i = 0 ; i < bits ; i++ ){
        if( i == 0 ) sumi[ i ] = bin1[ i ] ^ bin2[ i ] ^ 0 ;
        else sumi[ i ] = bin1[ i ] ^ bin2[ i ] ^ ci[ i - 1 ] ;
    }

    //PrintArray( sumi , BITS , "sumi" ) ;

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