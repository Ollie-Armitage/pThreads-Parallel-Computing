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

double **average_array(double **array, int DIMENSIONS){

    double** newArray = malloc(sizeof(double*) * DIMENSIONS);

    for(int i = 0; i < DIMENSIONS; i++){
        newArray[i] = malloc(sizeof(double*) * DIMENSIONS);
    }

    for(int i = 0; i < DIMENSIONS; i++){
            for(int j = 0; j < DIMENSIONS; j++) {
                if (j != 0 && j != DIMENSIONS - 1 && i != 0 && i != DIMENSIONS - 1) {
                    newArray[i][j] = average(array[i + 1][j], array[i - 1][j], array[i][j + 1], array[i][j - 1]);
                }
                else{
                    newArray[i][j] = array[i][j];
                }
            }

    }
    return newArray;
}

int main(int argc, char** argv) {
    char* tempPointer;
    int NUM_THREADS = 0;
    int DIMENSIONS = 5;
    double PRECISION = strtod(argv[3], &tempPointer);
    double** values = generate_r_array(DIMENSIONS);
    print_array(values, DIMENSIONS);
    sscanf(argv[1], "%d", &NUM_THREADS);
    printf("%d\n", NUM_THREADS);


    int RUNNING = 1;

    while(RUNNING) {
        double **newAverage = average_array(values, DIMENSIONS);
        print_array(newAverage, DIMENSIONS);

        if(IN_PRECISION(values, newAverage, DIMENSIONS, PRECISION)){ RUNNING = 0;}
        else{ values = newAverage;}
    }


    printf("MAIN program has ended.\n");
    return 0;
}