#include<stdio.h> 
#include<stdlib.h>

int main( int argc , char ** argv )
{
    // overflow: 32768*32768*2 ;
    int num = 32768*32768 ;
    printf( "%d\n" , num ) ;
}
