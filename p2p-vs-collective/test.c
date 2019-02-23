#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main( int argc , char ** argv ) {

    int my_size = 16 ;
    printf( "my_size: %f\n" , (double) my_size ) ;
    double ans ;
    ans = log10( (double) my_size  ) ;
    printf( "ans: %f\n" , ans ) ;

    return EXIT_SUCCESS ;
}
