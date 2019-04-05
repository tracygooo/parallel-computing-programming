/*
 * Test function for setting parameters via arguments passed from command line
 */

#include<stdio.h> 
#include<stdlib.h>

int BGQ_nodes_num = 4 ;
int rank_per_node = 64 ;
int number_of_ticks = 256 ;
int THRESHOLD = 25 ;
char FNAME[ 100 ] ;
double given_threshold = 0.25 ;

void SetParameters( char ** arg_vec ) ;
void PrintGlobalVar() ;

int main(int argc, char *argv[])
{
    printf( "Original parameters are:\n" ) ;
    //PrintGlobalVar() ;
    SetParameters( argv ) ;
    printf( "\n\nNew parameters are:\n" ) ;
    PrintGlobalVar() ;
}

void SetParameters( char ** arg_vec ) {
    BGQ_nodes_num = atoi( arg_vec[ 1 ] ) ;
    rank_per_node = atoi( arg_vec[ 2 ] ) ;
    number_of_ticks = atoi( arg_vec[ 3 ] ) ;
    given_threshold = atof( arg_vec[ 4 ] ) ;
    sprintf( FNAME , "%d.txt" , rank_per_node ) ;
}

void PrintGlobalVar() {
    printf( "BGQ_nodes_num = %d\n" , BGQ_nodes_num ) ;
    printf( "rank_per_node = %d\n" , rank_per_node ) ;
    printf( "number_of_ticks = %d\n" , number_of_ticks ) ;
    printf( "given_threshold = %f\n" , given_threshold ) ;
    printf( "FNAME = %s\n" , FNAME ) ;
}
