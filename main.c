#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>

// First argument should be Number of Threads.strtod(argv[3], &PRECISION);

int main(int argc, char** argv) {

    char* tempPointer;
    int NUM_THREADS = 0;
    int DIMENSIONS = 0;
    double PRECISION = 0;


    sscanf(argv[1], "%d", &NUM_THREADS);
    sscanf(argv[2], "%d", &DIMENSIONS);
    PRECISION = strtod(argv[3], tempPointer);
    double VALUES[DIMENSIONS][DIMENSIONS];

    printf("%ld\n", PRECISION);



    printf("%d\n", NUM_THREADS);


    printf("MAIN program has ended.\n");
    return 0;
}