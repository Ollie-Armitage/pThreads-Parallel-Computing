
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

struct args{
    int THREAD_NUMBER;
    int DIMENSIONS;
    int POSITION;
    int ROWS_PER_THREAD;
    int REMAINDER;
    int ALLOCATED_THREADS;
    double** END_ARRAY;
    double** PREV_ARRAY;
    pthread_t *THREAD_IDS;
};

pthread_rwlock_t ELEMENT_LOCK;



double PRECISION = 0;
int MAX_THREADS = 0;

// 0 For Threaded, 1 For Sequentially.

int RUN_TYPE = 0;

double **generate_random_array(int DIMENSIONS);
double ** allocate_double_array(int DIMENSIONS);
double ** solver(double** INPUT_ARRAY, int THREAD_NUM, int DIMENSIONS);
double average(double UP, double DOWN, double LEFT, double RIGHT);
void dismantle_array(double** ARRAY, int DIMENSIONS);
void set_array_edges(double** RANDOM_ARRAY, double** AVERAGED_VALUES, int DIMENSIONS);
void print_double_array(double **array, int DIMENSIONS);
void copy_array_section(double ** TO_ARRAY, double ** FROM_ARRAY, int FIRST_ROW, int FINAL_ROW, int DIMENSIONS);
int check_precision_section(double ** END_ARRAY, double ** PREVIOUS_ARRAY, int FIRST_ROW, int FINAL_ROW, int DIMENSIONS);
void* runnable(void* args);


int IN_PRECISION();


int main(int argc, char** argv){
    // Format Args.

    pthread_rwlock_init(&ELEMENT_LOCK, NULL);

    int DIMENSIONS = 0;
    int CORRECTNESS_TESTING = 0;


    sscanf(argv[1], "%d", &MAX_THREADS);
    sscanf(argv[2], "%d", &DIMENSIONS);
    char *temp_pointer;
    PRECISION = strtod(argv[3], &temp_pointer);

    if(argv[4] != NULL){
        sscanf(argv[4], "%d", &CORRECTNESS_TESTING);
    }

    if(MAX_THREADS == 1){
        RUN_TYPE = 1;
    }


    // Setup Double Array.

    double** INPUT_ARRAY = generate_random_array(DIMENSIONS);

    print_double_array(INPUT_ARRAY, DIMENSIONS);

    double** END_ARRAY = solver(INPUT_ARRAY, MAX_THREADS, DIMENSIONS);

    print_double_array(END_ARRAY, DIMENSIONS);




    return 0;
}

// Doesn't need to be locked.

void print_double_array(double **array, int DIMENSIONS){
    for(int i = 0; i < DIMENSIONS; i++){
        for(int j = 0; j < DIMENSIONS; j++){
            printf("%f ", array[i][j]);
        }
        printf("\n");
    }
    printf("\n\n");
}

// Doesn't need to be locked.

double ** allocate_double_array(int DIMENSIONS){ double** array = malloc(sizeof(double) * DIMENSIONS);
    for(int i = 0; i < DIMENSIONS; i++){
        array[i] = calloc(DIMENSIONS, sizeof(double) * DIMENSIONS);
    }
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

void dismantle_array(double** ARRAY, int DIMENSIONS){
    for(int i = 0; i < DIMENSIONS; i++){
        free(ARRAY[i]);
    }
}

double ** solver(double** PREVIOUS_ARRAY, int THREAD_NUM, int DIMENSIONS){


    pthread_t *THREAD_IDS = malloc(sizeof(pthread_t)*MAX_THREADS);
    struct args args;
    args.THREAD_NUMBER = 0;
    args.DIMENSIONS = DIMENSIONS;
    args.END_ARRAY = allocate_double_array(DIMENSIONS);
    args.PREV_ARRAY = PREVIOUS_ARRAY;
    args.THREAD_IDS = THREAD_IDS;
    args.POSITION = 1;
    args.ROWS_PER_THREAD = (DIMENSIONS - 2) / THREAD_NUM;
    args.REMAINDER = (DIMENSIONS - 2) % THREAD_NUM;
    args.ALLOCATED_THREADS = THREAD_NUM;

    set_array_edges(args.PREV_ARRAY, args.END_ARRAY, DIMENSIONS);

    runnable(&args);

    return args.END_ARRAY;
}

double average(double UP, double DOWN, double LEFT, double RIGHT){
   return (UP + DOWN + LEFT + RIGHT) / 4;
}

int check_precision_section(double ** END_ARRAY, double ** PREVIOUS_ARRAY, int FIRST_ROW, int FINAL_ROW, int DIMENSIONS){

    for(int i = FIRST_ROW; i < FINAL_ROW; i++){
        for(int j = 1; j < DIMENSIONS - 1; j++){
            if(PREVIOUS_ARRAY[i][j] - END_ARRAY[i][j] > PRECISION || PREVIOUS_ARRAY[i][j] - END_ARRAY[i][j] < -PRECISION){
                return 0;
            }
        }
    }

    return 1;

}

void copy_array_section(double ** TO_ARRAY, double ** FROM_ARRAY, int FIRST_ROW, int FINAL_ROW, int DIMENSIONS){
    int i = 0;
    for(i = FIRST_ROW; i < FINAL_ROW; i++){
        pthread_rwlock_wrlock(&ELEMENT_LOCK);
        memcpy(TO_ARRAY[i], FROM_ARRAY[i], sizeof(double)*DIMENSIONS);
        pthread_rwlock_unlock(&ELEMENT_LOCK);
    }
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

void* runnable(void* args){

    struct args ARGS = *(struct args*)args;
    int DIMENSIONS = ARGS.DIMENSIONS;
    int THREAD_NUMBER = ARGS.THREAD_NUMBER;
    int ROWS_PER_THREAD = ARGS.ROWS_PER_THREAD;
    int FIRST_ROW = ARGS.POSITION;
    int FINAL_ROW = FIRST_ROW + ROWS_PER_THREAD;
    int ALLOCATED_THREADS = ARGS.ALLOCATED_THREADS;
    int REMAINDER = ARGS.REMAINDER;
    double ** END_ARRAY = ARGS.END_ARRAY;
    double ** PREVIOUS_ARRAY = ARGS.PREV_ARRAY;
    pthread_t * THREAD_IDS =  ARGS.THREAD_IDS;
    int PRECISION_FLAG = 0;
    int REMAINDER_FLAG = 0;
    double ** BUFFER = malloc(sizeof(double)*DIMENSIONS);

    // When to make another thread.

    if(THREAD_NUMBER < ALLOCATED_THREADS - 1){

        // Setup Args for next thread.

        ARGS.POSITION = ARGS.POSITION + ROWS_PER_THREAD;
        ARGS.THREAD_NUMBER++;

        pthread_create(&THREAD_IDS[THREAD_NUMBER], NULL, runnable, &ARGS);
    }

    while(!PRECISION_FLAG) {
        for (int ROW = FIRST_ROW; ROW < FINAL_ROW; ROW++) {
            for (int COLUMN = 1; COLUMN < (DIMENSIONS - 1); COLUMN++) {


                double reads = 0;


                // Critical Section. Don't need row's.

                if (RUN_TYPE == 0) { pthread_rwlock_rdlock(&ELEMENT_LOCK); }
                double LEFT = PREVIOUS_ARRAY[ROW - 1][COLUMN];
                double RIGHT = PREVIOUS_ARRAY[ROW + 1][COLUMN];
                double DOWN = PREVIOUS_ARRAY[ROW][COLUMN - 1];
                double UP = PREVIOUS_ARRAY[ROW][COLUMN + 1];
                if (RUN_TYPE == 0) { pthread_rwlock_unlock(&ELEMENT_LOCK); }

                reads = average(UP, DOWN, LEFT, RIGHT);
                END_ARRAY[ROW][COLUMN] = reads;
            }
        }


        if (THREAD_NUMBER + 1 == ALLOCATED_THREADS) {
            for (int ROW = FINAL_ROW; ROW < (DIMENSIONS - 1); ROW++) {
                for (int COLUMN = 1; COLUMN < (DIMENSIONS - 1); COLUMN++) {

                    double UP;
                    double DOWN;
                    double reads;

                    double LEFT = PREVIOUS_ARRAY[ROW - 1][COLUMN];
                    double RIGHT = PREVIOUS_ARRAY[ROW + 1][COLUMN];
                    if (RUN_TYPE == 0) pthread_rwlock_rdlock(&ELEMENT_LOCK);
                    DOWN = PREVIOUS_ARRAY[ROW][COLUMN - 1];
                    UP = PREVIOUS_ARRAY[ROW][COLUMN + 1];
                    if (RUN_TYPE == 0) pthread_rwlock_unlock(&ELEMENT_LOCK);

                    reads = average(UP, DOWN, LEFT, RIGHT);
                    END_ARRAY[ROW][COLUMN] = reads;
                    REMAINDER_FLAG = 1;


                }
            }
        }


            PRECISION_FLAG = check_precision_section(END_ARRAY, PREVIOUS_ARRAY, FIRST_ROW, FINAL_ROW, DIMENSIONS);
            copy_array_section(PREVIOUS_ARRAY, END_ARRAY, FIRST_ROW, FINAL_ROW, DIMENSIONS);

            if (REMAINDER_FLAG == 1) {
                PRECISION_FLAG = check_precision_section(END_ARRAY, PREVIOUS_ARRAY, FINAL_ROW, DIMENSIONS - 1,
                                                         DIMENSIONS);
                copy_array_section(PREVIOUS_ARRAY, END_ARRAY, FINAL_ROW, DIMENSIONS - 1, DIMENSIONS);
                REMAINDER_FLAG = 0;
            }

    }

    if (THREAD_NUMBER + 1 != ALLOCATED_THREADS) {
        pthread_join(THREAD_IDS[THREAD_NUMBER], NULL);
    }

}





