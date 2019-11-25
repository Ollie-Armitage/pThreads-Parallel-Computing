
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>

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
pthread_mutex_t ARRAY_LOCK;
pthread_barrier_t WAIT;
pthread_barrier_t LEAVE_barrier;

double PRECISION = 0;
int PRECISION_FLAG = 0;
int MAX_THREADS = 0;


// 0 For Threaded, 1 For Sequentially.
int RUN_TYPE = 0;


pthread_mutex_t PRECISION_LOCK;


double **generate_random_array(int DIMENSIONS);
double **generate_standard_array(int DIMENSIONS);
double ** allocate_double_array(int DIMENSIONS);
double ** solver(double** INPUT_ARRAY, int THREAD_NUM, int DIMENSIONS);
double average(double UP, double DOWN, double LEFT, double RIGHT);
void dismantle_array(double** ARRAY, int DIMENSIONS);
void set_array_edges(double** RANDOM_ARRAY, double** AVERAGED_VALUES, int DIMENSIONS);
void print_double_array(double **array, int DIMENSIONS);
void copy_array_section(double ** TO_ARRAY, double ** FROM_ARRAY, int FIRST_ROW, int FINAL_ROW, int DIMENSIONS);
int check_precision_section(double ** END_ARRAY, double ** PREVIOUS_ARRAY, int FIRST_ROW, int FINAL_ROW, int DIMENSIONS);
void* runnable(void* args);

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

    if(CORRECTNESS_TESTING == 1){
        double** INPUT_ARRAY = generate_standard_array(DIMENSIONS);
        //double ** SINGLE_THREAD_SOLVER = solver(INPUT_ARRAY, 1, DIMENSIONS);
        //dismantle_array(INPUT_ARRAY, DIMENSIONS);
        //INPUT_ARRAY = generate_standard_array(DIMENSIONS);
        print_double_array(INPUT_ARRAY, DIMENSIONS);
        double ** MULTI_THREAD_SOLVER = solver(INPUT_ARRAY, MAX_THREADS, DIMENSIONS);

        //print_double_array(SINGLE_THREAD_SOLVER, DIMENSIONS);
        print_double_array(MULTI_THREAD_SOLVER, DIMENSIONS);

        //if(check_precision_section(SINGLE_THREAD_SOLVER, MULTI_THREAD_SOLVER, 0, DIMENSIONS, DIMENSIONS)){
        //    printf("Single Thread and Multi thread solver's return answers within precision of eachother.\n");
        //}
    }
    else{
        double** INPUT_ARRAY = generate_standard_array(DIMENSIONS);
        double** END_ARRAY = solver(INPUT_ARRAY, MAX_THREADS, DIMENSIONS);
        print_double_array(END_ARRAY, DIMENSIONS);
        dismantle_array(INPUT_ARRAY, DIMENSIONS);
        dismantle_array(END_ARRAY, DIMENSIONS);
    }


    return 0;
}

// Doesn't need to be locked.

void print_double_array(double **array, int DIMENSIONS){
    int i;
    for(i = 0; i < DIMENSIONS; i++){
        int j;
        for(j = 0; j < DIMENSIONS; j++){
            printf("%f\t", array[i][j]);
        }
        printf("\n");
    }
    printf("\n\n");
}

// Doesn't need to be locked.

double ** allocate_double_array(int DIMENSIONS){ double** array = malloc(sizeof(double) * DIMENSIONS);
    int i;
    for(i = 0; i < DIMENSIONS; i++){
        array[i] = calloc(DIMENSIONS, sizeof(double) * DIMENSIONS);
    }
    return array;
}

// Generate a random array of double values between 0 and 10.

double **generate_random_array(int DIMENSIONS){
    srand(time(NULL));
    double** array = allocate_double_array(DIMENSIONS);
    double random_value;
    int i;
    for(i = 0; i < DIMENSIONS; i++){
        int j;
        for(j = 0; j < DIMENSIONS; j++){
            random_value = (double)rand()/RAND_MAX*10;
            array[i][j] = random_value;
        }
    }
    return array;
}

// Generate Standard Array.

double **generate_standard_array(int DIMENSIONS){
    double** array = allocate_double_array(DIMENSIONS);
    int i;
    for(i = 0; i < DIMENSIONS; i++){
        int j;
        for(j = 0; j < DIMENSIONS; j++){
            if(j%2){ array[i][j] = (i + 1)%10;}
            else{ array[i][j] = (i + 2)%10;}
        }
    }
    return array;
}

void dismantle_array(double** ARRAY, int DIMENSIONS){
    int i;
    for(i = 0; i < DIMENSIONS; i++){
        free(ARRAY[i]);
    }
}

double ** solver(double** PREVIOUS_ARRAY, int THREAD_NUM, int DIMENSIONS){

    // Changes how the program should be run. If there's only one thread running, Locks are unnecessary and so blocked.


    pthread_t *THREAD_IDS = malloc(sizeof(pthread_t)*THREAD_NUM);
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
    if(args.ALLOCATED_THREADS >= (DIMENSIONS - 2)) args.ALLOCATED_THREADS = (DIMENSIONS - 2);

    if(MAX_THREADS == 1){
        RUN_TYPE = 1;
    }else{
        pthread_rwlock_init(&ELEMENT_LOCK, NULL);
        pthread_mutex_init(&PRECISION_LOCK, NULL);
        pthread_barrier_init(&WAIT, NULL, args.ALLOCATED_THREADS);
        pthread_barrier_init(&LEAVE_barrier, NULL, args.ALLOCATED_THREADS);


    }


    set_array_edges(args.PREV_ARRAY, args.END_ARRAY, DIMENSIONS);

    runnable(&args);

    return args.END_ARRAY;
}

double average(double UP, double DOWN, double LEFT, double RIGHT){
   return (UP + DOWN + LEFT + RIGHT) / 4;
}

int check_precision_section(double ** END_ARRAY, double ** PREVIOUS_ARRAY, int FIRST_ROW, int FINAL_ROW, int DIMENSIONS){
    int i;
    for(i = FIRST_ROW; i < FINAL_ROW; i++){
        int j;
        for(j = 1; j < DIMENSIONS - 1; j++){
            if(PREVIOUS_ARRAY[i][j] - END_ARRAY[i][j] > PRECISION || PREVIOUS_ARRAY[i][j] - END_ARRAY[i][j] < -PRECISION){
                return 0;
            }
        }
    }
    return 1;
}

int CHECK_FLAG(){
    pthread_mutex_lock(&PRECISION_LOCK);
    int FLAG = PRECISION_FLAG;
    pthread_mutex_unlock(&PRECISION_LOCK);
    return FLAG;
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
    int i;
    for(i = 0; i < DIMENSIONS; i++){
        AVERAGED_VALUES[i][0] = RANDOM_ARRAY[i][0];
        AVERAGED_VALUES[i][DIMENSIONS-1] = RANDOM_ARRAY[i][DIMENSIONS-1];
    }

    int j;
    for(j = 1; j < DIMENSIONS-1; j++){
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
    double ** END_ARRAY = ARGS.END_ARRAY;
    double ** PREVIOUS_ARRAY = ARGS.PREV_ARRAY;
    pthread_t * THREAD_IDS =  ARGS.THREAD_IDS;

    int REMAINDER_FLAG = 0;



    printf("Entering Thread: %d\n", THREAD_NUMBER);


    // When to make another thread.

    if(THREAD_NUMBER < ALLOCATED_THREADS - 1){

        // Setup Args for next thread.

        ARGS.POSITION = ARGS.POSITION + ROWS_PER_THREAD;
        ARGS.THREAD_NUMBER++;

        pthread_create(&THREAD_IDS[THREAD_NUMBER], NULL, runnable, &ARGS);
    }

    while(1) {



        int ROW;
        for (ROW = FIRST_ROW; ROW < FINAL_ROW; ROW++) {
            int COLUMN;
            for (COLUMN = 1; COLUMN < (DIMENSIONS - 1); COLUMN++) {


                double reads = 0;


                // Critical Section. Don't need row's.

                if(RUN_TYPE == 0){pthread_rwlock_rdlock(&ELEMENT_LOCK);}
                double LEFT = PREVIOUS_ARRAY[ROW-1][COLUMN];
                double RIGHT = PREVIOUS_ARRAY[ROW+1][COLUMN];
                double DOWN =  PREVIOUS_ARRAY[ROW][COLUMN - 1];
                double UP = PREVIOUS_ARRAY[ROW][COLUMN + 1];
                if(RUN_TYPE == 0){pthread_rwlock_unlock(&ELEMENT_LOCK);}

                reads = average(UP, DOWN, LEFT, RIGHT);
                END_ARRAY[ROW][COLUMN] = reads;
            }
        }


        if(THREAD_NUMBER + 1 == ALLOCATED_THREADS){

            for(ROW = FINAL_ROW; ROW < (DIMENSIONS - 1); ROW++){
                int COLUMN;
                for(COLUMN = 1; COLUMN < (DIMENSIONS - 1); COLUMN++){
                    double reads = 0;
                    if(RUN_TYPE == 0){pthread_rwlock_rdlock(&ELEMENT_LOCK);}
                    double LEFT = PREVIOUS_ARRAY[ROW-1][COLUMN];
                    double RIGHT = PREVIOUS_ARRAY[ROW+1][COLUMN];
                    double DOWN =  PREVIOUS_ARRAY[ROW][COLUMN - 1];
                    double UP = PREVIOUS_ARRAY[ROW][COLUMN + 1];
                    if(RUN_TYPE == 0){ pthread_rwlock_unlock(&ELEMENT_LOCK);}

                    reads = average(UP, DOWN, LEFT, RIGHT);
                    END_ARRAY[ROW][COLUMN] = reads;
                    REMAINDER_FLAG = 1;
                }
            }
        }



        // All threads need to have done their rows before copying them into the PREVIOUS_ARRAY so that other averages
        // are not effected.

        if(THREAD_NUMBER == 0){
            PRECISION_FLAG = check_precision_section(END_ARRAY, PREVIOUS_ARRAY, 1, DIMENSIONS - 1, DIMENSIONS);
        }


        if(RUN_TYPE == 0){

            pthread_barrier_wait(&WAIT);

            if(CHECK_FLAG()){
                printf("Thread: %d escaped\n", THREAD_NUMBER);

                break;
            }
        }
        else{
            if(CHECK_FLAG()){
                printf("Thread: %d escaped\n", THREAD_NUMBER);

                break;
            }
        }

        if(THREAD_NUMBER == 0){
            copy_array_section(PREVIOUS_ARRAY, END_ARRAY, 1, DIMENSIONS-1, DIMENSIONS);
        }


        if(REMAINDER_FLAG == 1){
            PRECISION_FLAG = check_precision_section(END_ARRAY, PREVIOUS_ARRAY, FINAL_ROW, DIMENSIONS - 1, DIMENSIONS);
            copy_array_section(PREVIOUS_ARRAY, END_ARRAY, FINAL_ROW, DIMENSIONS - 1, DIMENSIONS);
            REMAINDER_FLAG = 0;
        }



    }

    if(THREAD_NUMBER + 1 != ALLOCATED_THREADS){
        pthread_join(THREAD_IDS[THREAD_NUMBER], NULL);
    }



    printf("Thread Exiting: %d\n", THREAD_NUMBER);

}




