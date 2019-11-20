//
// Created by ollie on 19/11/2019.
//

#include <stdio.h>
#include <stdlib.h>


int main(){
    FILE *in_file;
    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    FILE *out_file = fopen("averagedLog.txt", "a");

    in_file = fopen("log.txt", "r");

    if(in_file == NULL){
        exit(EXIT_FAILURE);
    }


    int counter = 0;
    int DIMENSIONS = 1;
    int THREADS = 1;

    double VALUES = 0;



    while((read = getline(&line, &len, in_file)) != -1){
        counter++;

        if(counter == 10){
            fprintf(out_file, "%d\t", THREADS);
            fprintf(out_file, "%d\t", DIMENSIONS);

            fprintf(out_file, "%lf\n", VALUES / 10);
            THREADS++;

            if(THREADS == 13){
                DIMENSIONS++;
                THREADS = 1;
                VALUES = 0;

            }

            counter = 0;
        }
        double temp = 0;

        sscanf(line, "%lf", &temp);

        VALUES = VALUES + temp;

    }



    fclose(in_file);
    fclose(out_file);
    if(line) free(line);
    exit(EXIT_SUCCESS);


}