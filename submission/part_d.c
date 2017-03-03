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

//this file combines results of two arrays
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

//read in vals and put them in an array
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

//spawn kids iteratively
int *iterative_spawn(int kids, int data_length, int * vals){
    FILE *output_file = NULL;
    const char * output_name = "part_d_output.txt";

    int *master_stats; 
    master_stats = malloc(3*sizeof(int));

    int *delete_me;
    delete_me = master_stats;

    //dummy vals to start out
    master_stats[0] = 0;
    master_stats[1] = INT_MAX;
    master_stats[2] = INT_MIN;

    pid_t pid;

    pid = fork();

    if(pid !=0){//this is the parent

        int i = 0;
        int elems_left = data_length;
        
        int start = 0;
        int end;

        //while the parent still has kids to spawn
        while((i<kids)){
            
            int child_pipe[2];

            if(pipe(child_pipe)){
                perror("pipe()");
                exit(1);
            }
            
            //find out how many to calculate
            double temp_elems = floor(elems_left/(kids-i));
            int this_elems = (int) temp_elems;

            end = start + this_elems;

            //figure out how many elems left to calc
            int temp = elems_left - this_elems;
            elems_left = temp;
            
            pid = fork();
            if(pid !=0){//if it's the parent

                //printf("this is parent, in the iterative func. id is %d, child is %d\n",(int) getpid(),(int) pid);
                
                int istream_parent;
                istream_parent = child_pipe[0];
                close(child_pipe[1]);

                int child_stats[3];

                //get results from kid
                read(istream_parent,child_stats,3*sizeof(int));

                //collate answers
                int *temp  = collate(master_stats,child_stats);

                master_stats = temp;
                i++;

            }else{//this is the kid

                output_file = fopen(output_name,"a");
                
                fprintf(output_file, "Hi, I'm process %d and my parent is %d.\n", (int)getpid(), (int)getppid());
                fclose(output_file);

                //printf("this is child. id is %d\n", (int) getpid());

                int ostream_child;
                ostream_child = child_pipe[1];
                close(child_pipe[0]);

                //calc my vals
                int * my_stats = stats(start,end,vals);
                
                //write results to parent
                write(ostream_child,my_stats,3*sizeof(int));
                i++;
                exit(1);
            }
            start = end;    
        }

    }else{
        exit(1);
    }

    free(delete_me);
    return master_stats;
    
}

//spawn kids recursively
int * recursive_spawn(int kids_left, int max_kids, int max_size, int *vals){

    FILE *output_file = NULL;
    const char * output_name = "part_d_output.txt";
    
    int child_pipe[2];

    if(pipe(child_pipe)){
        perror("pipe");
        exit(1);
    }

    //if still kids to be spawned
    if (kids_left>0){

        pid_t child_pid;

        child_pid = fork();
        if(child_pid == -1){
            perror("fork()");
            exit(-1);
        }

        if(child_pid!=0){//this is the parent
            output_file = fopen(output_name,"a");
            fprintf(output_file, "Hi, I'm process %d and my parent is %d.\n", (int)getpid(), (int)getppid());
            fclose(output_file);

            //printf("this is parent, in the recursive func for my results. id is %d, child is %d\n",(int) getpid(), (int) child_pid);
            
            int istream_parent;
            istream_parent = child_pipe[0];        
            close(child_pipe[1]);            

            //figure out how many elems this one will handle
            int this_elems = (int) floor(max_size/(kids_left+1));

            //how many left to pass 
            int elems_left = max_size - this_elems;

            int *new_elems = malloc(this_elems*sizeof(int));

            //get results for parent
            for(int i= 0;i<this_elems;i++){
                new_elems[i] = vals[i];
            }
            int * test = stats(0,this_elems,vals);
            
            int kids_to_spawn;

            //edge case makes sure all kids have work to do
            if(this_elems<max_kids){
                kids_to_spawn = 1;
            }else{
                kids_to_spawn = max_kids+1;
            }

            //get my results, with my kids doing the work!
            int * parent_stats = iterative_spawn(kids_to_spawn,this_elems,new_elems);
            free(new_elems);

            //int all_to_send[3];   
            int next_elems = (int) floor(elems_left/(kids_left));            
            int child_stats[3];            

            //read results from kid
            read(istream_parent,child_stats,3*sizeof(int));
            int * results = collate(parent_stats, child_stats);

            //printf("I'm parent %d, and I'm returning sum: %d, min: %d, max: %d,",(int)getpid(),results[0],results[1],results[2]);
            return results;

        }else{//this is the child
                   
            //printf("this is child. id is %d\n", (int) getpid());
            int ostream_child;
            ostream_child = child_pipe[1];      
            close(child_pipe[0]);            

            //how many elems I am calculating?
            int this_elems = (int) floor(max_size/(kids_left+1));

            //size of the subarray
            int elems_left = max_size-this_elems;
            int next_elems = (int) floor(elems_left/(kids_left));

            //copies subarray to be passed to next function call
            int *next_vals = malloc(elems_left*sizeof(int));
            memcpy(next_vals, &vals[this_elems],elems_left*sizeof(int));
            
            //get results through recursive call
            int *child_stats = recursive_spawn(kids_left-1,max_kids,elems_left,next_vals);
            
            //write results upstream
            write(ostream_child,child_stats,3*sizeof(int));
            
            free(next_vals);
            exit(1);

        }
    }else{//this is the base case

        output_file = fopen(output_name,"a");
        fprintf(output_file, "Hi, I'm process %d and my parent is %d.\n", (int)getpid(), (int)getppid());
        fclose(output_file); 

        //figure out how many elems to operate on
        int this_elems = max_size;
        
        int *test = stats(0,this_elems,vals);

        int kids_to_spawn;

        //make this edge case so processes have at least one piece of data
        if(this_elems<max_kids){
            kids_to_spawn = 1;
        }else{
            kids_to_spawn = max_kids+1;
        }

        //get my results, by having my kids the work for me!
        int * parent_stats = iterative_spawn(kids_to_spawn,this_elems,vals);       

        //printf("this is parent edge case recursive, id is %d\n",(int)getpid());

        //write results upstream to parent
        int ostream_child;
        ostream_child = child_pipe[1];        
        close(child_pipe[0]);

        return parent_stats;
    }
    
}


int main(int argc, char *argv[]){

    const char * output_name = "part_d_output.txt";

    FILE *output_file = NULL;
    output_file = fopen(output_name,"w");
    fclose(output_file);

    //open up files
    int kids = 3;
    for (int file_size = 1; file_size<=5; file_size++){

        FILE *ofp;
        char outputFilename[] = "part_d_output.txt";
        ofp = fopen(outputFilename,"a");

        fprintf (ofp, "For the list of size 10^%d:\n",file_size);
        fclose(ofp);
        //printf("For the list of size 10^%d:\n",file_size);

        int data_length = pow(10,file_size);

        int *vals = readFile(file_size,data_length);
        //now vals contains vals. everything after this can be split!

        //performing statistical operations on data
        clock_t begin = clock();
        int *answer = recursive_spawn(kids,kids,data_length,vals);
        clock_t end = clock();

        //get timings
        double time_spent = (double)(end-begin)/CLOCKS_PER_SEC;

        int sum = answer[0];
        int min = answer[1];
        int max = answer[2];

        /*
        printf("Max=%d\n", max);
        printf("Min=%d\n", min);  
        printf("Sum=%d\n", sum);
        printf("Time=%fsec\n\n",time_spent);*/

        //print answers
        ofp = fopen(outputFilename,"a");
        fprintf(ofp, "Max=%d\n", max);
        fprintf(ofp, "Min=%d\n", min); 
        fprintf(ofp, "Sum=%d\n", sum);
        fprintf(ofp, "Time=%fsec\n\n",time_spent);
        fclose(ofp);
        free (vals);
    }
        
    return 0;
}