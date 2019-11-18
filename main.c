#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

// First argument should be Number of Threads;




// Feed 2d Array into main > For each element that isn't a side element, make the same element in a different array
// equal the average of it's surrounding elements >  perform the same action on the next array.

struct args{
    double** array;
    double** newArray;
    int DIMENSIONS;
    int startRow;
    int endRow;
};

double average(double a, double b, double c, double d){
    return (a + b + c + d) / 4;
}

double **generate_r_array(int DIMENSIONS){
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

int IN_PRECISION(double **oldArray, double **newArray, int DIMENSIONS, double PRECISION){
    for(int i = 0; i < DIMENSIONS; i++){
        for(int j = 0; j < DIMENSIONS; j++){
            if(oldArray[i][j] - newArray[i][j] > PRECISION || oldArray[i][j] - newArray[i][j] < -PRECISION){
                return 0;
            }
        }
    }
    return 1;
}

void print_array(double **array, int DIMENSIONS){
    for(int i = 0; i < DIMENSIONS; i++){
        for(int j = 0; j < DIMENSIONS; j++){
            printf("%f ", array[i][j]);
        }
        printf("\n");
    }
    printf("\n\n");
}

// Averages a chunk of the array.

void* average_chunk(void* chunkArgs){
    double** array = ((struct args*)chunkArgs)->array;
    double ** newArray = ((struct args*)chunkArgs)->newArray;
    int DIMENSIONS = ((struct args*)chunkArgs)->DIMENSIONS;
    int startRow = ((struct args*)chunkArgs)->startRow;
    int endRow = ((struct args*)chunkArgs)->endRow;

    for(int i = startRow; i < endRow; i++){
        for(int j = 0; j < DIMENSIONS; j++){
            if (j != 0 && j != DIMENSIONS - 1 && i != 0 && i != DIMENSIONS - 1) {
                newArray[i][j] = average(array[i + 1][j], array[i - 1][j], array[i][j + 1], array[i][j - 1]);
            }
            else{
                newArray[i][j] = array[i][j];
            }
        }
    }
    pthread_exit(0);
}


double **average_array(double **array, int DIMENSIONS, int THREAD_NUM){

    double** newArray = malloc(sizeof(double*) * DIMENSIONS);

    for(int i = 0; i < DIMENSIONS; i++){
        newArray[i] = malloc(sizeof(double*) * DIMENSIONS);
    }

    // Thread ID:
    pthread_t tids[THREAD_NUM];

    //Create Attributes
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    int i = 0;
    while(i < DIMENSIONS){
        struct args *chunkArgs = malloc(sizeof(struct args));
        chunkArgs->array = array;
        chunkArgs->newArray = newArray; // The whole array, meaning it'll get rewritten every time.
        chunkArgs->DIMENSIONS = DIMENSIONS;
        chunkArgs->startRow = i;
        chunkArgs->endRow = i+(DIMENSIONS / THREAD_NUM);




        printf("Creating Thread: %lu\n", tids[i]);
        pthread_create(&tids[i], &attr, average_chunk, (void *)chunkArgs);
        i = i + 1;
    }






    return newArray;
}

// Things that can be run in parallel: 1. Averaging chunks of the array. 2. Checking the precision of the array

int main(int argc, char** argv) {
    char* tempPointer;
    int NUM_THREADS = 0;
    int DIMENSIONS = 3;
    double PRECISION = strtod(argv[3], &tempPointer);
    double** values = generate_r_array(DIMENSIONS);
    print_array(values, DIMENSIONS);
    sscanf(argv[1], "%d", &NUM_THREADS);


    int RUNNING = 1;

    while(RUNNING) {
        double **newAverage = average_array(values, DIMENSIONS, NUM_THREADS);
        print_array(newAverage, DIMENSIONS);

        if(IN_PRECISION(values, newAverage, DIMENSIONS, PRECISION)){ // If precision is run in parallel, arrays need to be localized to in_precision before values = newAverage.
            RUNNING = 0;
        }
        else{ values = newAverage;}
    }



    printf("MAIN program has ended.\n");
    return 0;
}