#include <stdio.h>
//for i/o

#include <stdlib.h>
#include <string.h>
//for strcat, strcpy

#include <limits.h>
//int_max, int_min

#include <math.h>
//pow function, for file_size

#include <time.h>
//for timing

//need to link to math '-lm'
//gcc -std=c99 -o objectname filename.c -lm

//function that calcs sum, max, min, of range of nums in array
int* stats(int start, int end, int * array){
    int temp_sum = 0;
    int temp_max = INT_MIN;
    int temp_min = INT_MAX;

    for(int i = start;i<=end; i++){
            int curr = array[i];
            temp_sum += curr;
            if (curr<temp_min){
                temp_min = curr;
            }
            if  (curr>temp_max){
                temp_max = curr;
            }
        }

    static int results[3];
    results[0] = temp_sum;
    results[1] = temp_min;
    results[2] = temp_max;

    return results;
}

int * readFile(int file, int data_size){

    const char * file_pref = "./input/input_file_10^";
    const char * file_suff = ".txt";

    char file_name[48];

    char temp[2];
    sprintf(temp,"%d",file);

    //concat'ing file name
    strcpy(file_name, file_pref);
    strcat(file_name, temp);
    strcat(file_name,file_suff);   

    //dynamically allocating array
    static int* output_vals;
    output_vals = malloc(data_size*sizeof(int));

    //this is opening file
    FILE *fp = NULL;
    fp = fopen(file_name,"r");

    //checking fopen() success
    if (fp == NULL){
        printf("\nWarning, fopen() failed!\n");
        int * error;
        *error = -1;
        return error;
    }   

    //reading in data 
    for(int i =0;i<data_size;i++){
        fscanf(fp,"%d", &output_vals[i]);
    }

    //printf("Success! read in okay\n");

    fclose(fp);

    return output_vals;
}

int main(int argc, char *argv[]){

    const char * output_name = "./output/output_file_serial.txt";

    FILE *output_file = NULL;
    output_file = fopen(output_name,"w");

    for(int file_size = 1; file_size<7;file_size++){
   
        //int file_size = 1; //this is for testing the simple code        

        double ELEMS = pow(10,(double)file_size);
        int data_length = (int) ELEMS;

        int *vals = readFile(file_size,data_length);
        //now vals contains vals. everything after this can be split!

        //performing statistical operations on data

        clock_t begin = clock();

        //call function to get stats
        int * stat_array = stats(0,data_length-1,vals);

        int sum = stat_array[0];
        int min = stat_array[1];
        int max = stat_array[2];
        //printf("results of function call: %d sum, %d sum, %d sum", stat_array[0],stat_array[1],stat_array[2]);

        clock_t end = clock();
        double time_spent = (double)(end-begin)/CLOCKS_PER_SEC;
        

        //printf("fscanf() successful!\n");

        
        //printf("\n File stream closed through fclose()!\n");

        char output[128];

        sprintf(output,"File of size 10^%d has:\nSum: %d,\n min: %d,\n max: %d,\n time: %fsec\n\n",file_size,sum, min,max, time_spent);
        fputs(output,output_file);
        
    }
    return 0;
}