#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<math.h>
#include<mpi.h>
#include<pthread.h>

pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;

void * Mysum( void * c ) ;
int main(int argc, char *argv[])
{
    int mpi_commsize ;
    int mpi_myrank ;
    MPI_Init( &argc, &argv);
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_commsize);
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_myrank);


    int num_threads = 5/mpi_commsize - 1 ;
    int cnt[ 2 ] = { 0 , 0 } ;
    pthread_t p_threads[ num_threads ]; 
    pthread_attr_t attr; 
    pthread_attr_init (&attr); 

    pthread_mutex_init( & count_mutex , NULL ) ;


    int i ;
    for( i = 0 ; i < num_threads ; i++ ) {
        pthread_create( &p_threads[ i ], &attr, Mysum , cnt ); 
    }

    Mysum( cnt ) ;

    for (i = 0; i < num_threads; i++)
    {
        pthread_join(p_threads[i], NULL);

    }
    pthread_mutex_destroy( & count_mutex ) ;

    if( mpi_myrank == 0 ) {
        //printf( "cnt[ 0 ] = %d\n" , cnt[ 0 ] ) ;
    }

    MPI_Finalize();
}

void * Mysum( void * c )
{
    int * my_cnt = (int * ) c ;
    int delta = 10 ;
    int i ;
    for( i = 0 ; i < 5 ; i++ ) {
        //pthread_mutex_trylock( & count_mutex ) ;
        pthread_mutex_lock( & count_mutex ) ;
        my_cnt[ 0 ] += i ;
        pthread_mutex_unlock( & count_mutex ) ;
    }
    printf( "my_cnt[0] = %d\n" , my_cnt[ 0 ] ) ;

    return NULL ;
} 
