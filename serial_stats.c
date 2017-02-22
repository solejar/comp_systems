#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <time.h>
//need to link to math '-lm'
//gcc -std=c99 -o objectname filename.c -lm

int main(int argc, char *argv[]){

    //these are parts of filename
    const char * file_pref = "./input/input_file_10^";
    const char * file_suff = ".txt";

    const char * output_name = "./output/output_file_serial.txt";

    FILE *output_file = NULL;
    output_file = fopen(output_name,"w");

    for(int file_size = 1; file_size<7;file_size++){
        
        //this is construction of filename
        //printf("Got this far with i of %d\n",file_size);
        char file_name[48];

        char temp[2];
        sprintf(temp,"%d",file_size);

        strcpy(file_name, file_pref);
        strcat(file_name, temp);
        strcat(file_name,file_suff);

        double SIZE = 4;
        //number of elems
        
        double ELEMS = pow(10,(double)file_size);
        int data_length = (int) ELEMS;

        int totalBytes = (int) SIZE*ELEMS;
        //printf("Total Bytes SHOULD be %i\n", totalBytes);

        int vals[data_length];

        //this is opening file

        FILE *fp = NULL;
        fp = fopen(file_name,"r");

        //checking fopen() success
        if (fp == NULL){
            printf("\nWarning, fopen() failed!\n");
            return 1;
        }   

        
        //printf("doing %d loops on %s\n", data_length,file_name);

        //reading in data 
        for(int i =0;i<data_length;i++){
            fscanf(fp,"%d", &vals[i]);
        }

        //now vals contains vals. everything after this can be split!
        int sum = 0;
        int min = INT_MAX;
        int max = INT_MIN;

        //performing statistical operations on data

        clock_t begin = clock();

        for(int i = 0;i<data_length; i++){
            int curr = vals[i];
            sum += curr;
            if (curr<min){
                min = curr;
            }
            if  (curr>max){
                max = curr;
            }
        }

        clock_t end = clock();
        double time_spent = (double)(end-begin)/CLOCKS_PER_SEC;


        //printf("fscanf() successful!\n");

        fclose(fp);
        //printf("\n File stream closed through fclose()!\n");

        char output[128];

        sprintf(output,"File of size 10^%d has:\nSum: %d,\n min: %d,\n max: %d,\n time: %fsec\n\n",file_size,sum, min,max, time_spent);
        fputs(output,output_file);

    }
    return 0;
}