#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <semaphore.h>

// First argument should be Number of Threads;




// Feed 2d Array into main > For each element that isn't a side element, make the same element in a different array
// equal the average of it's surrounding elements >  perform the same action on the next array.

struct args{
    double** array;
    double* row;
    int rowNumber;
}Args;

pthread_barrier_t barrier;

// ARGS
int NUM_THREADS;
int DIMENSIONS;
double PRECISION;

double average(double a, double b, double c, double d){
    return (a + b + c + d) / 4;
}

double **generate_r_array(){
    srand(time(NULL));
    double** array;

    array = malloc(sizeof(double*) * DIMENSIONS);

    for(int i = 0; i < DIMENSIONS; i++){

        array[i] = malloc(sizeof(double*) * DIMENSIONS);

        for(int j = 0; j < DIMENSIONS; j++){
            double random_value;
            random_value = (double)rand()/RAND_MAX*6.0;
            array[i][j] = random_value;

        }
    }

    return array;
}

int IN_PRECISION(double **oldArray, double **newArray){
    for(int i = 0; i < DIMENSIONS; i++){
        for(int j = 0; j < DIMENSIONS; j++){
            if(oldArray[i][j] - newArray[i][j] > PRECISION || oldArray[i][j] - newArray[i][j] < -PRECISION){
                return 0;
            }
        }
    }
    return 1;
}

void print_array(double **array){
    for(int i = 0; i < DIMENSIONS; i++){
        for(int j = 0; j < DIMENSIONS; j++){
            printf("%f ", array[i][j]);
        }
        printf("\n");
    }
    printf("\n\n");
}

// Averages a row of the array.

void* average_row(void* chunkArgs){
    int rowNumber = ((struct args*)chunkArgs)->rowNumber;
    double** array = ((struct args*)chunkArgs)->array;
    double * newArray = ((struct args*)chunkArgs)->row;

    for(int i = 0; i < DIMENSIONS; i++){

            if (i != 0 && i != DIMENSIONS - 1 && rowNumber != 0 && rowNumber != DIMENSIONS - 1) {
                newArray[i] = average(array[i + 1][rowNumber], array[i - 1][rowNumber], array[i][rowNumber + 1], array[i][rowNumber - 1]);
            }
            else{
                newArray[i] = array[rowNumber][i];
            }

    }
    free(chunkArgs);
    pthread_barrier_wait(&barrier);
}

double **average_array(double **array, int allotedThreads){


    // Memory allocated to averaged array that's going to be returned.

    double** newArray = malloc(sizeof(double*) * DIMENSIONS);
    for(int i = 0; i < DIMENSIONS; i++){
        newArray[i] = malloc(sizeof(double*) * DIMENSIONS);
    }

    // Thread IDs:
    pthread_t *thread_ids = malloc(sizeof(pthread_t)*allotedThreads);

    // For each chunk of workload, allocate each thread a row. Remaining work is then done after.

    pthread_barrier_init(&barrier, NULL, allotedThreads+1);

    struct args *rowArgs = NULL;

    for(int i = 0; i < allotedThreads; i++){
        rowArgs = malloc(sizeof(struct args));
        rowArgs->array = array;
        rowArgs->rowNumber = i;
        rowArgs->row = newArray[i];
        pthread_create(&thread_ids[i], NULL, average_row, rowArgs);
    }

    pthread_barrier_wait(&barrier);

    for(int i = 0; i < allotedThreads; i++){
        pthread_join(thread_ids[i], NULL);
    }

    free(thread_ids);
    return newArray;
}

// Things that can be run in parallel: 1. Averaging chunks of the array. 2. Checking the precision of the array

int main(int argc, char** argv) {
    char* tempPointer;
    NUM_THREADS = 0;
    DIMENSIONS = 8;
    PRECISION = strtod(argv[3], &tempPointer);
    double** values = generate_r_array();
    print_array(values);
    sscanf(argv[1], "%d", &NUM_THREADS);


    int RUNNING = 1;

    while(RUNNING) {
        double **newAverage = average_array(values, NUM_THREADS);
        print_array(newAverage);

        if(IN_PRECISION(values, newAverage)){// If precision is run in parallel, arrays need to be localized to in_precision before values = newAverage.

            for(int i = 0; i < DIMENSIONS; i++){
                double *ptr = values[i];
                double *ptr2 = newAverage[i];
                free(ptr);
                free(ptr2);
            }

            free(values);
            free(newAverage);
            RUNNING = 0;
        }
        else{

            for(int i = 0; i < DIMENSIONS; i++){
                double *ptr = values[i];
                free(ptr);
            }

            values = newAverage;

        }
    }



    printf("MAIN program has ended.\n");
    return 0;
}