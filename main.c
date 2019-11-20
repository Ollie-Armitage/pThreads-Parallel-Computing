#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <time.h>

// Structure of arguments to be fed into a thread for it's function.

struct args{
    double** ARRAY;
    double** END_VALUES;
    int START_ROW;
    int END_ROW;
    int DIMENSIONS;
};

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

double** get_volatile_values(double **ARRAY, int DIMENSIONS){
    double ** VOLATILE_VALUES = malloc(sizeof(double)*DIMENSIONS-2);

    for(int i = 1; i < DIMENSIONS-1; i++){
        VOLATILE_VALUES[i-1] = ARRAY[i] + 1;
    }

    return VOLATILE_VALUES;

}

double average(double a, double b, double c, double d){
    return (a + b + c + d) / 4;
}


void* average_array_chunk(void* chunkArgs){

    int START_ROW = ((struct args*)chunkArgs)->START_ROW;
    int END_ROW = ((struct args*)chunkArgs)->END_ROW;
    int DIMENSIONS = ((struct args*)chunkArgs)->DIMENSIONS;
    double** ARRAY = ((struct args*)chunkArgs)->ARRAY;

    double** END_VALUES = ((struct args*)chunkArgs)->END_VALUES;

    for(int i = START_ROW; i < END_ROW; i++){
        for(int j = 1; j < DIMENSIONS-1; j++){
            END_VALUES[i][j] = average(ARRAY[i - 1][j], ARRAY[i + 1][j], ARRAY[i][j - 1], ARRAY[i][j + 1]);
        }
    }
    free(chunkArgs);

}

double** sequential_solver(int DIMENSIONS, double** RANDOM_ARRAY, double** AVERAGED_VALUES, double PRECISION){
    // do the program sequentially.

    struct args *chunkArgs;

    chunkArgs = malloc(sizeof(struct args));
    chunkArgs->START_ROW = 1;
    chunkArgs->END_ROW = DIMENSIONS - 1;
    chunkArgs->DIMENSIONS = DIMENSIONS;
    chunkArgs->ARRAY = RANDOM_ARRAY;
    chunkArgs->END_VALUES = AVERAGED_VALUES;

    average_array_chunk(chunkArgs);

    while(1){

        chunkArgs = malloc(sizeof(struct args));
        chunkArgs->START_ROW = 1;
        chunkArgs->END_ROW = DIMENSIONS - 1;
        chunkArgs->DIMENSIONS = DIMENSIONS;
        chunkArgs->ARRAY = RANDOM_ARRAY;
        chunkArgs->END_VALUES = AVERAGED_VALUES;

        if(IN_PRECISION(RANDOM_ARRAY, AVERAGED_VALUES, PRECISION, DIMENSIONS)){
            return AVERAGED_VALUES;
        }else{

            for(int i = 1; i < DIMENSIONS-1; i++){
                for(int j = 1; j < DIMENSIONS-1; j++){
                    RANDOM_ARRAY[i][j] = AVERAGED_VALUES[i][j];
                }
            }
            average_array_chunk(chunkArgs);
        }
    }
}



double** parallel_solver(int DIMENSIONS, int NUM_THREADS, double** RANDOM_ARRAY, double** AVERAGED_VALUES, double PRECISION){
    struct args *chunkArgs;
    int AVERAGING_THREADS = NUM_THREADS - 1;

    // If the number of threads allocated to the program are greater than the dimensions of the array, reduce to dimension size.
    while(1) {

        if(IN_PRECISION(RANDOM_ARRAY, AVERAGED_VALUES, PRECISION, DIMENSIONS)){
            return AVERAGED_VALUES;
        }
        else{

            for(int i = 1; i < DIMENSIONS-1; i++){
                for(int j = 1; j < DIMENSIONS-1; j++){
                    RANDOM_ARRAY[i][j] = AVERAGED_VALUES[i][j];
                }
            }

            if (AVERAGING_THREADS > (DIMENSIONS - 2)) {
                AVERAGING_THREADS = DIMENSIONS - 2;
            }

            int WORK_PER_THREAD = (DIMENSIONS - 2) / AVERAGING_THREADS;
            pthread_t *ids = malloc(sizeof(pthread_t) * AVERAGING_THREADS);


            int THREAD_COUNTER = 0;

            for (int i = 1; i < DIMENSIONS - 1 && THREAD_COUNTER < AVERAGING_THREADS; i = i + WORK_PER_THREAD) {

                chunkArgs = malloc(sizeof(struct args));
                chunkArgs->START_ROW = i;
                chunkArgs->END_ROW = ((struct args *) chunkArgs)->START_ROW + WORK_PER_THREAD;
                chunkArgs->DIMENSIONS = DIMENSIONS;
                chunkArgs->ARRAY = RANDOM_ARRAY;
                chunkArgs->END_VALUES = AVERAGED_VALUES;

                pthread_create(&ids[THREAD_COUNTER++], NULL, average_array_chunk, chunkArgs);

            }

            int REMAINDER = (DIMENSIONS - 2) % AVERAGING_THREADS;

            for (int i = 0; i < THREAD_COUNTER; i++) {
                pthread_join(ids[i], NULL);


                if (REMAINDER > 0) {

                    chunkArgs = malloc(sizeof(struct args));
                    chunkArgs->START_ROW = (DIMENSIONS - 1) - REMAINDER;
                    chunkArgs->END_ROW = DIMENSIONS - 1;
                    chunkArgs->DIMENSIONS = DIMENSIONS;
                    chunkArgs->ARRAY = RANDOM_ARRAY;
                    chunkArgs->END_VALUES = AVERAGED_VALUES;

                    pthread_create(&ids[i], NULL, average_array_chunk, chunkArgs);
                    REMAINDER = 0;
                    pthread_join(ids[i], NULL);
                }

            }

            free(ids);
        }
    }
}

// Returns the averaged array of values that is within precision.
double** solver(double** RANDOM_ARRAY, int DIMENSIONS, int NUM_THREADS, double PRECISION){

    double ** AVERAGED_VALUES = allocate_double_array(DIMENSIONS);
    set_array_edges(RANDOM_ARRAY, AVERAGED_VALUES, DIMENSIONS);


    if(NUM_THREADS == 1){
        sequential_solver(DIMENSIONS, RANDOM_ARRAY, AVERAGED_VALUES, PRECISION);
    }
    else{
        parallel_solver(DIMENSIONS, NUM_THREADS, RANDOM_ARRAY, AVERAGED_VALUES, PRECISION);
    }
    return AVERAGED_VALUES;

}


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