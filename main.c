
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>



struct args{
    int THREAD_NUMBER;
    int DIMENSIONS;
    int POSITION;
    int ROWS_REMAINING;
    int ROWS_PER_THREAD;
    double** END_ARRAY;
    double** PREV_ARRAY;
    pthread_t *THREAD_IDS;
};

pthread_mutex_t LOCK;

double PRECISION = 0;
int PRECISION_FLAG = 0;
int MAX_THREADS = 0;

double **generate_random_array(int DIMENSIONS);
double ** allocate_double_array(int DIMENSIONS);
double ** solver(double** INPUT_ARRAY, int THREAD_NUM, int DIMENSIONS);
double average(double UP, double DOWN, double LEFT, double RIGHT);
void dismantle_array(double** ARRAY, int DIMENSIONS);
void print_double_array(double **array, int DIMENSIONS);
void copy_array(double ** FROM_ARRAY, double ** TO_ARRAY, int DIMENSIONS);
void CHECK_PRECISION(double ** END_ARRAY, double ** PREVIOUS_ARRAY, int DIMENSIONS);
void* runnable(void* args);


int IN_PRECISION();


int main(int argc, char** argv){
    // Format Args.

    int DIMENSIONS = 0;
    int CORRECTNESS_TESTING = 0;

    sscanf(argv[1], "%d", &MAX_THREADS);
    sscanf(argv[2], "%d", &DIMENSIONS);
    char *temp_pointer;
    PRECISION = strtod(argv[3], &temp_pointer);

    if(argv[4] != NULL){
        sscanf(argv[4], "%d", &CORRECTNESS_TESTING);
    }

    // Setup Double Array.

    double** INPUT_ARRAY = generate_random_array(DIMENSIONS);
    double ** SINGLE_THREAD_ARRAY = NULL;

    //print_double_array(INPUT_ARRAY);


    if(CORRECTNESS_TESTING == 1){
        SINGLE_THREAD_ARRAY = allocate_double_array(DIMENSIONS);
        copy_array(INPUT_ARRAY, SINGLE_THREAD_ARRAY, DIMENSIONS);
        solver(SINGLE_THREAD_ARRAY, 1, DIMENSIONS);
        print_double_array(SINGLE_THREAD_ARRAY, DIMENSIONS);
    }

    print_double_array(INPUT_ARRAY, DIMENSIONS);


    solver(INPUT_ARRAY, MAX_THREADS, DIMENSIONS);


    //print_double_array(INPUT_ARRAY);

    if(CORRECTNESS_TESTING == 1){
        CHECK_PRECISION(SINGLE_THREAD_ARRAY, INPUT_ARRAY, DIMENSIONS);

        if(IN_PRECISION()){
            printf("In Precision!\n");
        }

    }

    return 0;
}

void print_double_array(double **array, int DIMENSIONS){
    for(int i = 0; i < DIMENSIONS; i++){
        for(int j = 0; j < DIMENSIONS; j++){
            printf("%f ", array[i][j]);
        }
        printf("\n");
    }
    printf("\n\n");
}

double ** allocate_double_array(int DIMENSIONS){ double** array = malloc(sizeof(double) * DIMENSIONS);
    for(int i = 0; i < DIMENSIONS; i++){ array[i] = calloc(DIMENSIONS, sizeof(double) * DIMENSIONS);}
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

    pthread_mutex_init(&LOCK, NULL);

    pthread_t *THREAD_IDS = malloc(sizeof(pthread_t)*MAX_THREADS);

    struct args args;
    args.THREAD_NUMBER = 1;
    args.DIMENSIONS = DIMENSIONS;
    args.END_ARRAY = allocate_double_array(DIMENSIONS);
    args.PREV_ARRAY = PREVIOUS_ARRAY;
    args.THREAD_IDS = THREAD_IDS;
    args.POSITION = 1;
    args.ROWS_PER_THREAD = (DIMENSIONS - 2) / THREAD_NUM;
    args.ROWS_REMAINING = DIMENSIONS - 2;

    runnable(&args);

    return args.END_ARRAY;
}

double average(double UP, double DOWN, double LEFT, double RIGHT){
   return (UP + DOWN + LEFT + RIGHT) / 4;
}

void UPDATE_PRECISION_FLAG(int UPDATE){
    PRECISION_FLAG = UPDATE;
}

void CHECK_PRECISION(double ** END_ARRAY, double ** PREVIOUS_ARRAY, int DIMENSIONS){

    for(int i = 0; i < DIMENSIONS; i++){
        for(int j = 0; j < DIMENSIONS; j++){
            if(PREVIOUS_ARRAY[i][j] - END_ARRAY[i][j] > PRECISION || PREVIOUS_ARRAY[i][j] - END_ARRAY[i][j] < -PRECISION){
                UPDATE_PRECISION_FLAG(0);
                return;
            }
        }
    }

    UPDATE_PRECISION_FLAG(1);

}

int IN_PRECISION(){
    return PRECISION_FLAG;
}

void copy_array(double ** FROM_ARRAY, double ** TO_ARRAY, int DIMENSIONS){
    for(int i = 0; i < DIMENSIONS; i++){
        for(int j = 0; j < DIMENSIONS; j++){
            TO_ARRAY[i][j] = FROM_ARRAY[i][j];
        }
    }
}

void* runnable(void* args){

    struct args ARGS = *(struct args*)args;
    int DIMENSIONS = ARGS.DIMENSIONS;
    int THREAD_NUMBER = ARGS.THREAD_NUMBER++;
    int ROWS_PER_THREAD = ARGS.ROWS_PER_THREAD;
    int ROWS_REMAINING = ARGS.ROWS_REMAINING;
    int POSITION = ARGS.POSITION;
    double ** END_ARRAY = ARGS.END_ARRAY;
    double ** PREVIOUS_ARRAY = ARGS.PREV_ARRAY;
    pthread_t * THREAD_IDS =  ARGS.THREAD_IDS;



    printf("Entering Thread: %d\n", THREAD_NUMBER);

    if(THREAD_NUMBER < MAX_THREADS){
        ARGS.ROWS_REMAINING = ARGS.ROWS_REMAINING - ROWS_PER_THREAD;
        ARGS.POSITION = ARGS.POSITION + ROWS_PER_THREAD;
        pthread_create(&THREAD_IDS[ARGS.THREAD_NUMBER], NULL, runnable, &ARGS);
    }

    int FIRST_ROW = POSITION;
    int FINAL_ROW = POSITION + ROWS_PER_THREAD;

    while(!IN_PRECISION()) {

        for (int ROW = FIRST_ROW; ROW < FINAL_ROW; ROW++) {
            for (int COLUMN = 1; COLUMN < (DIMENSIONS - 1); COLUMN++) {
                // Critical Section.
                END_ARRAY[ROW][COLUMN] = average(PREVIOUS_ARRAY[ROW][COLUMN + 1], PREVIOUS_ARRAY[ROW][COLUMN - 1], PREVIOUS_ARRAY[ROW + 1][COLUMN], PREVIOUS_ARRAY[ROW - 1][COLUMN]);

            }
        }

        // TODO: REMAINING ROWS.

        if(POSITION == DIMENSIONS - 2 - ROWS_REMAINING){
            for(int ROW = DIMENSIONS - ROWS_PER_THREAD; ROW < DIMENSIONS - 1; ROW++){
                for(int COLUMN = 1; COLUMN < (DIMENSIONS - 1); COLUMN++){
                    END_ARRAY[ROW][COLUMN] = average(PREVIOUS_ARRAY[ROW][COLUMN + 1], PREVIOUS_ARRAY[ROW][COLUMN - 1], PREVIOUS_ARRAY[ROW + 1][COLUMN], PREVIOUS_ARRAY[ROW - 1][COLUMN]);
                }
            }
        }

        CHECK_PRECISION(END_ARRAY, PREVIOUS_ARRAY, DIMENSIONS);
        copy_array(END_ARRAY, PREVIOUS_ARRAY, DIMENSIONS);

    }

    if(THREAD_NUMBER == 1){
        pthread_mutex_destroy(&LOCK);
    }


    printf("Thread Exiting: %d\n", THREAD_NUMBER);

}




