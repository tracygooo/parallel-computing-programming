#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int PrintArray( const int * arr  , const int arr_size ) ;
int main()
{

    /*================================================
     * Test array addresss
    ================================================*/
    //int a[5] = {1,2,3,4,5} ;
    //printf( "a[4] = %d" ,*( a + 4 ) ) ;
    

    /*================================================
     * Test MPI_Scatter 
    ================================================*/
    int * a = NULL;
    int i;
    int local_a[6] ;
    int local_n ;
    int n = 6;
    int my_rank ;
    int my_mpi_size ;

    MPI_Init( NULL , NULL );
    MPI_Comm_size(MPI_COMM_WORLD, &my_mpi_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); 
    local_n = n / my_mpi_size ;

    if (my_rank == 0) { 
        a = malloc(n * sizeof(int));
        for (i = 0; i < n; i++)
            scanf( "%d", &a[i] ) ;
        MPI_Scatter( a, local_n, MPI_INT, local_a, local_n, MPI_INT, 0, MPI_COMM_WORLD) ;
        printf( "Rank %d: \n" , my_rank ) ;
        PrintArray( local_a , 2 ) ;
        free(a);
    } 

    else { 
        MPI_Scatter(a, local_n, MPI_INT, local_a, local_n, MPI_INT, 0, MPI_COMM_WORLD);
        printf( "Rank %d: \n" , my_rank ) ;
        PrintArray( local_a , 2 ) ;
    } 

    MPI_Finalize();

    return EXIT_SUCCESS ;
}

/****************************************
 *  Print out array of int
 *  ****************************************/
int PrintArray( const int * arr  , const int arr_size ) {
    int i ;
    for( i = 0 ; i < arr_size ; i++ ) {
        printf( "%d " , arr[i] ) ;
    }
    printf( "\n\n" ) ;

    return EXIT_SUCCESS ;
}

