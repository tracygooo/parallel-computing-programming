
/* wr_at.c
 *  *
 *   * Example to demonstrate use of MPI_File_write_at and MPI_File_read_at
 *    *
 *    */

#include <stdio.h>
#include "mpi.h"

#define NUM_INTS 100

void sample_error(int error, char *string)
{
    fprintf(stderr, "Error %d in %s\n", error, string);
    MPI_Finalize();
    exit(-1);
}

    void
main( int argc, char **argv )
{  
    char filename[128];
    int i, rank,  comm_size;
    int *buff1, *buff2;
    MPI_File fh;
    MPI_Offset disp, offset, file_size;
    MPI_Datatype etype, ftype, buftype;
    MPI_Info info;
    MPI_Status status;
    int result, count, differs;

    if(argc < 2) {
        fprintf(stdout, "Missing argument: filename\n");
        exit(-1);
    }
    strcpy(filename, argv[1]);

    MPI_Init(&argc, &argv);

    /* get this processor's rank */
    result = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if(result != MPI_SUCCESS) 
        sample_error(result, "MPI_Comm_rank");

    result = MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    if(result != MPI_SUCCESS) 
        sample_error(result, "MPI_Comm_size");

    /* communicator group MPI_COMM_WORLD opens file "foo" 
     *      for reading and writing (and creating, if necessary) */
    result = MPI_File_open(MPI_COMM_WORLD, filename, 
            MPI_MODE_RDWR | MPI_MODE_CREATE, (int)NULL, &fh);
    if(result != MPI_SUCCESS) 
        sample_error(result, "MPI_File_open");

    /* Set the file view which tiles the file type MPI_INT, starting 
     *      at displacement 0.  In this example, the etype is also MPI_INT.
     *       */
    disp = 0;
    etype = MPI_INT;
    ftype = MPI_INT;
    info = (MPI_Info)NULL;
    result = MPI_File_set_view(fh, disp, etype, ftype, (char *)NULL,
            info);
    if(result != MPI_SUCCESS) 
        sample_error(result, "MPI_File_set_view");

    /* Allocate and initialize a buffer (buff1) containing NUM_INTS
     * integers, 
     *      where the integer in location i is set to i. */
    buff1 = (int *)malloc(NUM_INTS*sizeof(int));
    for(i=0;i<NUM_INTS;i++) buff1[i] = i;

    /* Set the buffer type to also be MPI_INT, then write the buffer
     * (buff1)
     *      starting at offset 0, i.e., the first etype in the file. */ 
    buftype = MPI_INT;
    offset = rank * NUM_INTS;
    result = MPI_File_write_at(fh, offset, buff1, NUM_INTS, buftype, &status);
    if(result != MPI_SUCCESS) 
        sample_error(result, "MPI_File_write_at");

    result = MPI_Get_elements(&status, MPI_BYTE, &count);
    if(result != MPI_SUCCESS)
        sample_error(result, "MPI_Get_elements");
    if(count != NUM_INTS*sizeof(int))
        fprintf(stderr, "Did not write the same number of bytes as requested\n");
    else
        fprintf(stdout, "Wrote %d bytes\n", count);

    /* Allocate another buffer (buff2) to read into, then read NUM_INTS
     *      integers into this buffer.  */
    buff2 = (int *)malloc(NUM_INTS*sizeof(int));
    result = MPI_File_read_at(fh, offset, buff2, NUM_INTS, buftype, &status); 
    if(result != MPI_SUCCESS) 
        sample_error(result, "MPI_File_read_at");

    /* Find out how many bytes were read and compare to how many
     *      we expected */
    result = MPI_Get_elements(&status, MPI_BYTE, &count);
    if(result != MPI_SUCCESS)
        sample_error(result, "MPI_Get_elements");
    if(count != NUM_INTS*sizeof(int))
        fprintf(stderr, "Did not read the same number of bytes as requested\n");
    else
        fprintf(stdout, "Read %d bytes\n", count);

    /* Check to see that each integer read from each location is 
     *      the same as the integer written to that location. */
    differs = 0;
    for(i=0; i<NUM_INTS; i++) {
        if(buff1[i] != buff2[i]) {
            fprintf(stderr, "Integer number %d differs\n", i);
            differs = 1;
        }
    }
    if(!differs)
        fprintf(stdout, "Wrote and read the same data\n");

    MPI_Barrier(MPI_COMM_WORLD);

    result = MPI_File_get_size(fh, &file_size);
    if(result != MPI_SUCCESS) 
        sample_error(result, "MPI_File_get_size");

    /* Compare the file size with what we expect */
    /* To see a negative response, make the file preexist with a larger
     *      size than what is written by this program */
    if(file_size != (comm_size * NUM_INTS * sizeof(int)))
        fprintf(stderr, "File size is not equal to the write size\n");

    result = MPI_File_close(&fh);
    if(result != MPI_SUCCESS) 
        sample_error(result, "MPI_File_close");

    MPI_Finalize();

    free(buff1);
    free(buff2);
}
