#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<math.h>

#include<clcg4.h>

int main(int argc, char *argv[])
{
    InitDefault();
    int i ;
    for( i = 0 ; i < 10 ; i++ ) {
        printf("%dth Random Value of %f\n", i , GenVal( 1 ) ) ;
    }
}
