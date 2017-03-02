#include <stdio.h>
//for i/o

#include <stdlib.h>
#include <string.h>
//for strcat, strcpy and for memcpy

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

    for(int i = start;i<end; i++){
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

int *iterative_spawn(int kids, int data_length, int * vals){
    int *master_stats; //[3];
    master_stats = malloc(3*sizeof(int));

    int *delete_me;
    delete_me = master_stats;

    master_stats[0] = 0;
    master_stats[1] = INT_MAX;
    master_stats[2] = INT_MIN;

    pid_t pid;

    pid = fork();

    if(pid !=0){
        int i = 0;
        int elems_left = data_length;
        int start = 0;
        int end;

        while((i<kids)){
            //int start;
            int child_pipe[2];

            if(pipe(child_pipe)){
                perror("pipe()");
                exit(1);
            }
            
            //this gon floor by itself
            int this_elems = elems_left/(kids);
            end = start + this_elems;

            int elems_left = elems_left - this_elems;
            pid = fork();
            if(pid !=0){
                printf("this is parent. id is %d, child is %d\n",(int) getpid(),(int) pid);
                //int * parent_stats = stats()
                int istream_parent;
                istream_parent = child_pipe[0];

                close(child_pipe[1]);

                int * child_stats = malloc(3*sizeof(int));

                read(istream_parent,child_stats,3*sizeof(int));
                int *temp  = collate(master_stats,child_stats);

                free(child_stats);
                master_stats = temp;
                i++;

                
            }else{
                printf("this is child. id is %d\n", (int) getpid());

                int ostream_child;
                ostream_child = child_pipe[1];
                close(child_pipe[0]);

                int * my_stats = stats(start,end,vals);
                printf("my kiddy results are %d sum, %d min, %d max\n",my_stats[0],my_stats[1],my_stats[2]);
                write(ostream_child,my_stats,3*sizeof(int));
                i++;
                exit(1);
            }
            start = end;    
        }
    }else{
        exit(1);
    }

    return master_stats;
    free(delete_me);
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
        int kids = 3;

        int *master_stats = iterative_spawn(kids,data_length,vals);

        printf("results of function call: %d sum, %d min, %d max\n", master_stats[0],master_stats[1],master_stats[2]);
        //printf("this is definitely where the error is");
        
        //int *answer = spawn_children(3,data_length,vals);
        //printf("results of function call: %d sum, %d min, %d max", answer[0],answer[1],answer[2]);

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