#include <stdio.h>
#include <stdlib.h>

int ComputePower( const int base , const int exponent ) {
    int i ;
    int power = 1 ; 
    for( i = 0 ; i < exponent ; i ++ ) {
        power *= base ;
    }
    return power ;
}

int ComputeLog2( const int power ) {
    int exponent = 0 ;
    int tmp = 1 ;
    while( tmp < power ){
        tmp *= 2 ;
        exponent += 1 ;
    }
    return exponent ;

}

int main( int argc , char ** argv ) {

    // Test function ComputePower() 
    int base = 2 ; 
    int exponent = 1 ;
    printf( "%d^%d = %d\n" , base , exponent , ComputePower( base , exponent ) ) ;

    // Test function ComputeLog2() 
    int power = 1025 ;
    printf( "log2(%d) = %d\n" , power , ComputeLog2( power ) ) ;

    return EXIT_SUCCESS ;
}
