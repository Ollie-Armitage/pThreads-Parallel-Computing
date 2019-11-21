
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

struct args{
    int DIMENSIONS;
    int THREADS_AVAILABLE;
    int THREAD_NUMBER;
    double** END_ARRAY;
    pthread_mutex_t LOCK;
    pthread_mutex_t THREAD_LOCK;
    pthread_t *THREAD_IDS;
};

double PRECISION = 0;
int PRECISION_FLAG = 0;
int COUNT = 0;
pthread_mutex_t COUNT_LOCK;

double ** solver(double** INPUT_ARRAY, int NUM_THREADS, int DIMENSIONS, double PRECISION);
void dismantle_array(double** ARRAY, int DIMENSIONS);
double **generate_random_array(int DIMENSIONS);
void print_double_array(double **array, int DIMENSIONS);
double ** allocate_double_array(int SIZE);
void* runnable(void* args);


int main(int argc, char** argv){
    // Time Taken Setup.

    clock_t start, end;
    double cpu_time_used;
    start = clock();

    pthread_mutex_init(&COUNT_LOCK, NULL);

    // Format Args.

    int NUM_THREADS = 0;
    int DIMENSIONS = 0;
    sscanf(argv[1], "%d", &NUM_THREADS);
    sscanf(argv[2], "%d", &DIMENSIONS);
    char *temp_pointer;
    PRECISION = strtod(argv[3], &temp_pointer);

    // Account for the current thread.


    // Setup Double Array.

    double** INPUT_ARRAY = generate_random_array(DIMENSIONS);

    solver(INPUT_ARRAY, NUM_THREADS, DIMENSIONS, PRECISION);

    print_double_array(INPUT_ARRAY, DIMENSIONS);

    dismantle_array(INPUT_ARRAY, DIMENSIONS);

    // End time taken.

    end = clock();

    // Work out the time taken and file it.

    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

    /*FILE *out_file = fopen("log.txt", "a");

    if(out_file == NULL){
        printf("Couldn't open outfile.\n");
        exit(-1);
    }

    fprintf(out_file, "%f\n", cpu_time_used);
    fclose(out_file);*/

    printf("MAIN program has ended in %f, in ", cpu_time_used);
    printf("%d runs.\n", COUNT);


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

double ** allocate_double_array(int SIZE){
    double** array = malloc(sizeof(double)*SIZE);
    for(int i = 0; i < SIZE; i++){ array[i] = calloc(SIZE, sizeof(double)*SIZE);}
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
        double *tempPtr = ARRAY[i];
        free(tempPtr);
    }
}

void free_struct(struct args* args){
    if(args != NULL){
        free(args->THREAD_IDS);
        free(args);
    }
    args = NULL;
}

double ** solver(double** END_ARRAY, int NUM_THREADS, int DIMENSIONS, double PRECISION){


    pthread_mutex_t LOCK;
    pthread_mutex_init(&LOCK, NULL);
    pthread_mutex_t THREAD_LOCK;
    pthread_mutex_init(&THREAD_LOCK, NULL);
    pthread_t *THREAD_IDS = malloc(sizeof(pthread_t)*NUM_THREADS);

    struct args args;
    args.DIMENSIONS = DIMENSIONS;
    args.THREADS_AVAILABLE = NUM_THREADS - 1;
    args.THREAD_NUMBER = 1;
    args.LOCK = LOCK;
    args.THREAD_LOCK = THREAD_LOCK;
    args.END_ARRAY = END_ARRAY;
    args.THREAD_IDS = THREAD_IDS;

    runnable(&args);


    return END_ARRAY;
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

void INC_COUNT(){
    pthread_mutex_lock(&COUNT_LOCK);
    COUNT++;
    pthread_mutex_unlock(&COUNT_LOCK);
}

int IN_PRECISION(){
    INC_COUNT();
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
    struct args *pointer= (struct args*)args;
    struct args ARGS = *pointer;
    int THREAD_NUMBER = ARGS.THREAD_NUMBER;
    printf("Entering Thread: %d\n", THREAD_NUMBER);

    int DIMENSIONS = ARGS.DIMENSIONS;
    int THREADS_AVAILABLE = ARGS.THREADS_AVAILABLE;

    pthread_t * THREAD_IDS =  ARGS.THREAD_IDS;
    double ** END_ARRAY = ARGS.END_ARRAY;
    double** PREVIOUS_ARRAY = NULL;

    if(THREAD_NUMBER == 1){
         PREVIOUS_ARRAY = allocate_double_array(DIMENSIONS);
    }


    pthread_mutex_t lock = ARGS.LOCK;

    while(!IN_PRECISION()) {

        if(THREAD_NUMBER == 1){
            CHECK_PRECISION(END_ARRAY, PREVIOUS_ARRAY, DIMENSIONS);
            copy_array(END_ARRAY, PREVIOUS_ARRAY, DIMENSIONS);
        }

        // Horizontal.
        for (int ROW = 1; ROW < (DIMENSIONS - 1); ROW++) {
            for (int COLUMN = 1; COLUMN < (DIMENSIONS - 1); COLUMN++) {

                if (THREADS_AVAILABLE > 0 && ROW == 2 && COLUMN == 2) {

                    ARGS.THREADS_AVAILABLE = ARGS.THREADS_AVAILABLE - 1;
                    ARGS.THREAD_NUMBER = ARGS.THREAD_NUMBER + 1;

                    pthread_create(&THREAD_IDS[ARGS.THREAD_NUMBER], NULL, runnable, args);
                    THREADS_AVAILABLE = -1;
                }

                pthread_mutex_lock(&lock);
                END_ARRAY[ROW][COLUMN] = average(END_ARRAY[ROW][COLUMN + 1], END_ARRAY[ROW][COLUMN - 1],END_ARRAY[ROW + 1][COLUMN], END_ARRAY[ROW - 1][COLUMN]);
                pthread_mutex_unlock(&lock);


            }

        }

    }


    if(THREADS_AVAILABLE == -1) {
        pthread_join(THREAD_IDS[THREAD_NUMBER + 1], NULL);
    }

    if(THREAD_NUMBER == 1){
        dismantle_array(PREVIOUS_ARRAY, DIMENSIONS);
        free(PREVIOUS_ARRAY);
    }


    printf("Thread Exiting: %d\n", THREAD_NUMBER);

}




