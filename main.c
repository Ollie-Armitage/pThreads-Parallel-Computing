#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <time.h>

struct args;

double **generate_random_array(int DIMENSIONS);

void free_double_array(double** array, int DIMENSIONS);

void print_double_array(double **array, int DIMENSIONS);

void set_array_edges(double** RANDOM_ARRAY, double** AVERAGED_VALUES, int DIMENSIONS);

double ** allocate_double_array(int SIZE);

int IN_PRECISION(double **oldArray, double **newArray, double PRECISION, int DIMENSIONS);

double** solver(double** VALUES, int DIMENSIONS, int NUM_THREADS, double PRECISION);

int main(int argc, char** argv) {

    // Time Taken Setup.

    clock_t start, end;
    double cpu_time_used;
    start = clock();

    // Format Args.

    int NUM_THREADS = 0;
    int DIMENSIONS = 0;
    double PRECISION = 0;


    char* temp_pointer;

    sscanf(argv[1], "%d", &NUM_THREADS);
    sscanf(argv[2], "%d", &DIMENSIONS);
    PRECISION = strtod(argv[3], &temp_pointer);

    // Generate a random two-dimensional double array (Values between 0 - 10).

    double** VALUES = generate_random_array(DIMENSIONS);

    // Print Random Array.

    print_double_array(VALUES, DIMENSIONS);

    // Return the average of the values.

    double ** AVERAGED_VALUES = solver(VALUES, DIMENSIONS, NUM_THREADS, PRECISION);

    print_double_array(AVERAGED_VALUES, DIMENSIONS);

    // Free Random Array and the averaged values.

    free_double_array(VALUES, DIMENSIONS);
    free_double_array(AVERAGED_VALUES, DIMENSIONS);

    // End time taken.

    end = clock();

    printf("MAIN program has ended.\n");

    // Work out the time taken and file it.

    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    FILE *out_file = fopen("log.txt", "a");

    if(out_file == NULL){
        printf("Couldn't open outfile.\n");
        exit(-1);
    }

    fprintf(out_file, "%f\n", cpu_time_used);


    return 0;
}

void set_array_edges(double** RANDOM_ARRAY, double** AVERAGED_VALUES, int DIMENSIONS){

    // This could be parallelised.

    for(int i = 0; i < DIMENSIONS; i++){
        AVERAGED_VALUES[i][0] = RANDOM_ARRAY[i][0];
        AVERAGED_VALUES[i][DIMENSIONS-1] = RANDOM_ARRAY[i][DIMENSIONS-1];
    }

    for(int j = 1; j < DIMENSIONS-1; j++){
        AVERAGED_VALUES[0][j] = RANDOM_ARRAY[0][j];
        AVERAGED_VALUES[DIMENSIONS-1][j] = RANDOM_ARRAY[DIMENSIONS-1][j];
    }

}

// Returns the averaged array of values that is within precision.

double** solver(double** RANDOM_ARRAY, int DIMENSIONS, int NUM_THREADS, double PRECISION){
    double ** AVERAGED_VALUES = allocate_double_array(DIMENSIONS);

    set_array_edges(RANDOM_ARRAY, AVERAGED_VALUES, DIMENSIONS);

    return AVERAGED_VALUES;
}






// Structure of arguments to be fed into a thread for it's function.

struct args{
    double** array;
    double* row;
    int rowNumber;
};

double ** allocate_double_array(int SIZE){
    double** array = malloc(sizeof(double)*SIZE);
    for(int i = 0; i < SIZE; i++){ array[i] = calloc(SIZE, sizeof(double*)*SIZE);}
    return array;
}



// Generate a random array of double values between 0 and 10.

double **generate_random_array(int DIMENSIONS){
    srand(time(NULL));
    double** array = allocate_double_array(DIMENSIONS);
    double random_value;
    for(int i = 0; i < DIMENSIONS; i++){
        for(int j = 0; j < DIMENSIONS; j++){
            random_value = (double)rand()/RAND_MAX*10;
            array[i][j] = random_value;
        }
    }

    return array;
}

// Free's the memory used in an array of doubles.

void free_double_array(double** array, int DIMENSIONS){
    for(int i = 0; i < DIMENSIONS; i++){
        double *tempPtr = array[i];
        free(tempPtr);
    }
    free(array);
}

// Returns 1 if in the specified precision, 0 if not in the specified precision.

int IN_PRECISION(double **oldArray, double **newArray, double PRECISION, int DIMENSIONS){
    for(int i = 0; i < DIMENSIONS; i++){
        for(int j = 0; j < DIMENSIONS; j++){
            if(oldArray[i][j] - newArray[i][j] > PRECISION || oldArray[i][j] - newArray[i][j] < -PRECISION){
                return 0;
            }
        }
    }
    return 1;
}

// Prints an array of doubles.

void print_double_array(double **array, int DIMENSIONS){
    for(int i = 0; i < DIMENSIONS; i++){
        for(int j = 0; j < DIMENSIONS; j++){
            printf("%f ", array[i][j]);
        }
        printf("\n");
    }
    printf("\n\n");
}