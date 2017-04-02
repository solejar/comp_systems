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

//combines results of two arrays
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

//reads in file and puts it in array
int * readFile(int file, int data_size){

    const char * file_pref = "data_";
    const char * file_suff = ".txt";

    char inputFilename[24];
    sprintf(inputFilename,"data_%d.txt",file);

    //dynamically allocating array
    static int* output_vals;
    output_vals = malloc(data_size*sizeof(int));

    //this is opening file
    FILE *fp = NULL;
    fp = fopen(inputFilename,"r");

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

    fclose(fp);

    return output_vals;
}

//spawn the children iteratively
int * spawn_children_BFS(int total_children, int num_elem /*number of elements in vals */, int *vals /*input data*/){

    FILE *output_file = NULL;
    const char * output_name = "part_c_output.txt";
    output_file = fopen(output_name,"a");
    fprintf(output_file, "Hi, I'm parent %d.\n", (int)getpid());
    fclose(output_file);

    int *result[3];
    int kids_left = total_children;
    int numPerProc = num_elem/total_children;

    int *master_stats = malloc(3*sizeof(int));
    int *deleteMe = master_stats;

    //dummy placeholder
    master_stats[0] = 0;
    master_stats[1] = INT_MAX;
    master_stats[2] = INT_MIN;

    //collect results from all the kids
    for(int i = 0; i < total_children; i++){

        int pipefd[2];
        int child_stats[3];

        if(pipe(pipefd)){
            perror("pipe");
            exit(1);
        }

        pid_t pid = fork(); //create children

        if(pid > 0){ //its a parent

            close(pipefd[1]);
            //get results from kid
            read(pipefd[0],child_stats,3*sizeof(int));

            //combine running stats
            master_stats = collate(master_stats, child_stats);
            
        } 
        else if (pid == 0){ //otherwise its a kid

            if (i == (total_children - 1)){ //if its the last child
                output_file = fopen(output_name,"a");
                fprintf(output_file, "Hi, I'm process %d and my parent is %d.\n", (int)getpid(), (int)getppid());
                fclose(output_file);

                //write results upstream to parent
                close(pipefd[0]);
                write(pipefd[1], stats((i*numPerProc), num_elem-1, vals), (3*sizeof(int)));
                exit(0);
            }
            else{ //if its not the last child
                output_file = fopen(output_name,"a");
                fprintf(output_file, "Hi, I'm process %d and my parent is %d.\n", (int)getpid(), (int)getppid());
                fclose(output_file);

                //write results upstream to parent
                close(pipefd[0]);
                write(pipefd[1], stats((i*numPerProc), (i*numPerProc)+numPerProc-1, vals), (3*sizeof(int)));
                exit(0); 
            }
        } 
        else{
            perror("fork()");
            exit(-1);
        }
    }
    free(deleteMe);
    return master_stats;
    
}

int main(int argc, char *argv[]){

    const char * output_name = "part_c_output.txt";

    FILE *output_file = NULL;
    output_file = fopen(output_name,"w");
    fclose(output_file);

    //open up all files
    int kids = 3;
    for(int file_size = 1; file_size<=5;file_size++){

        output_file = fopen(output_name, "a");
    	fprintf (output_file, "For the list of size 10^%d:\n",file_size);
        fclose(output_file);

       	double ELEMS = pow(10,(double)file_size);
       	int data_length = (int) ELEMS;

        int *vals = readFile(file_size,data_length);
        //now vals contains vals. everything after this can be split!

        //performing statistical operations on data
        clock_t begin = clock();
        int *answer = spawn_children_BFS(kids,data_length,vals);
        clock_t end = clock();

        double time_spent = (double)(end-begin)/CLOCKS_PER_SEC;

        output_file = fopen(output_name, "a");
        fprintf(output_file, "Max = %d\nMin = %d\nSum = %d\nTime= %fsec\n\n", answer[2],answer[1],answer[0],time_spent);
        fclose(output_file);

        free (vals);
    }

    return 0;
}