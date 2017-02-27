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

#include <unistd.h>
//interact w/ processes

#include <sys/types.h>
//for pid_t

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

int * collate(int * array1, int *array2){
    int temp_sum = array1[0] + array2[0];
    int temp_min;
    int temp_max;

    static int output[3]; 

    if(array1[1]<array2[1]){
        temp_min = array1[1];
    }else{
        temp_min = array2[1];
    }

    if(array1[2]>array2[2]){
        temp_max = array1[2];
    }else{
        temp_max = array2[2];
    }

    output[0] = temp_sum;
    output[1] = temp_min;
    output[2] = temp_max;

    return output;
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

int * spawn_children(int kids_left){
    if (kids_left>0){

        int parent_pipe[2];
        int child_pipe[2];

        if(pipe(parent_pipe)||pipe(child_pipe)){
            perror("pipe");
            exit(1);
        }

        pid_t child_pid;

        child_pid = fork();

        if (child_pid == -1){
            perror("fork()");
            exit(1);
        }

        if(child_pid != 0){
            int istream_parent,ostream_parent;
            istream_parent = child_pipe[1];
            ostream_parent = parent_pipe[0];

            close(child_pipe[0]);
            close(parent_pipe[1]);

            printf("this is parent. id is %d\n",(int) getpid());
            printf("child id = %d\n", (int) child_pid);
        }
        else{
            int istream_child,ostream_child;
            istream_child = child_pipe[0];
            ostream_child = parent_pipe[1];

            close(child_pipe[1]);
            close(parent_pipe[0]);
            printf("this is child. id is %d\n", (int) getpid());
            //read in data, then stat it. then write results to parent.

            spawn_children(kids_left-1);
        }
    }else{
        //this is the leaf. 
        //read in data
        //write answer.
        //int my_answer = stat()
    }
}

int main(int argc, char *argv[]){

    const char * output_name = "./output/output_file_dfs.txt";

    FILE *output_file = NULL;
    output_file = fopen(output_name,"w");

    //for(int file_size = 1; file_size<7;file_size++){
    int file_size = 1; //this is for testing the simple code
        

        double ELEMS = pow(10,(double)file_size);
        int data_length = (int) ELEMS;

        int *vals = readFile(file_size,data_length);
        //now vals contains vals. everything after this can be split!

        //performing statistical operations on data

        spawn_children(2);
        /*
        //let's learn about pids!
        pid_t child_pid;

        child_pid = fork();
        if(child_pid != 0){
            printf("this is parent, cause fork returned nonzero. id is %d\n",(int) getpid());
            printf("child id = %d\n", (int) child_pid);
        }
        else{
            printf("this is child, cause return is 0. id is %d\n", (int) getpid());
        }*/

        /*
        clock_t begin = clock();

        //call function to get stats
        int * stat_array = stats(0,data_length-1,vals);

        int sum = stat_array[0];
        int min = stat_array[1];
        int max = stat_array[2];
        printf("results of function call: %d sum, %d min, %d max", stat_array[0],stat_array[1],stat_array[2]);
        
        clock_t end = clock();
        double time_spent = (double)(end-begin)/CLOCKS_PER_SEC;


        //printf("fscanf() successful!\n");

        
        //printf("\n File stream closed through fclose()!\n");

        char output[128];

        sprintf(output,"File of size 10^%d has:\nSum: %d,\n min: %d,\n max: %d,\n time: %fsec\n\n",file_size,sum, min,max, time_spent);
        fputs(output,output_file);
        */

    //}

    free (vals);
    return 0;
}