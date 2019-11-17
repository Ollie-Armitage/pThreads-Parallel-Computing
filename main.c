#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>

// First argument should be Number of Threads;

// Feed 2d Array into main > For each element that isn't a side element, make the same element in a different array
// equal the average of it's surrounding elements >  perform the same action on the next array.

double average(double a, double b, double c, double d){
    return (a + b + c + d) / 4;
}



int main(int argc, char** argv) {
    char* tempPointer;
    int NUM_THREADS = 4;
    int DIMENSIONS = 3;
    double PRECISION = strtod(argv[3], &tempPointer);
    double VALUES[DIMENSIONS][DIMENSIONS];
    double ANSWERS[DIMENSIONS][DIMENSIONS];
    pthread_t threads[NUM_THREADS];

    int totalValues = DIMENSIONS*DIMENSIONS;
    int valuesPerThread = totalValues / NUM_THREADS;

    srand(time(NULL));

    for(int i = 0; i < DIMENSIONS; i++){
        for(int j = 0; j < DIMENSIONS; j++){
            double random_value;
            random_value = (double)rand()/RAND_MAX*6.0;
            VALUES[i][j] = random_value;
            ANSWERS[i][j] = random_value;
            printf("%f ", VALUES[i][j]);
        }
        printf("\n");
        printf("\n");
    }



    double value = 0;

    for(int i = 1; i < DIMENSIONS; i++){
        for(int j = 1; j < DIMENSIONS; j++){
            value = average(VALUES[i+1][j], VALUES[i-1][j], VALUES[i][j+1], VALUES[i][j-1]);
            //printf("%f ", value);
            ANSWERS[i][j] = value;
        }
        //printf("\n");
    }


    // Print values to specified precisions.

    for(int i = 1; i < DIMENSIONS - 1; i++){
        for(int j = 1; j < DIMENSIONS - 1; j++){
            printf("%f ", floor(ANSWERS[i][j]/PRECISION)*PRECISION);
        }
        printf("\n");
    }




    sscanf(argv[1], "%d", &NUM_THREADS);


    printf("MAIN program has ended.\n");
    return 0;
}