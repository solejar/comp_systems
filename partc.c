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

int *readFile(int size){
	FILE *ifp;

	char inputFilename[12];
	sprintf(inputFilename, "data_%d.txt", size);

	ifp = fopen(inputFilename, "r");
	if (ifp == NULL) {
		fprintf(stderr, "Can't open input file %s!\n", inputFilename);
		exit(1);
	}

	int* arr;
	int length = pow(10,size);
	arr = malloc(length*sizeof(int));

	int i;
	for (i = 0; i < length; i++){
		fscanf(ifp, "%d", &arr[i]);
	}

	fclose(ifp);

	return arr;

}

int * spawn_children(int kids_left, int max_size, int *vals, FILE* ofp){
    //printf("spawn func called with %dkids_left, %dmax_size\n",kids_left,max_size);

    int pipefds[2];

    //printf("my kids_left is %d, if it's <=0, I expect to hit edge!\n",kids_left);

    if(pipe(pipefds)){
        perror("pipe");
        exit(1);
    }


    if (kids_left>0){

        //int parent_pipe[2];
        //int child_pipe[2];
        /*
        if(pipe(child_pipe)){
            perror("pipe");
            exit(1);
        }*/

        pid_t child_pid;

        child_pid = fork();
        if(child_pid == -1){
            perror("fork()");
            exit(-1);
        }
        if(child_pid!=0){
   //      	FILE *ofp;
			// char outputFilename[] = "part_b_outputSean2.txt";
			// ofp = fopen(outputFilename,"w");

   			printf ("Hi I'm process %d and my parent is %d.\n", (int) getpid(), (int) getppid());
			fprintf (ofp, "Hi I'm process %d and my parent is %d.\n", (int) getpid(), (int) getppid());
			//fclose(ofp);
            int istream_parent;
            istream_parent = pipefds[0];
            

            close(pipefds[1]);
            //close(parent_pipe[1]);

            //figure out how many elems this one will handle
            int this_elems = max_size/(kids_left+1);

            //how many left to pass 
            int elems_left = max_size - this_elems;

            //get results for parent
            int * parent_stats = stats(0, this_elems,vals);

            
            //int all_to_send[3];
            

            
            int child_stats[3];

            read(istream_parent,child_stats,3*sizeof(int));

            int * results = collate(parent_stats, child_stats);

            //printf("I'm parent %d, and I'm returning sum: %d, min: %d, max: %d,",(int)getpid(),results[0],results[1],results[2]);
            return results;

        }else{
            
            //printf("this is child. id is %d\n", (int) getpid());
            int ostream_child;
            ostream_child = pipefds[1];
            

            close(pipefds[0]);
            

            //floor might not be necessary
            int this_elems = max_size/(kids_left+1);

            //size of the subarray
            int elems_left = max_size-this_elems;


            //copies subarray to be passed to next function call
            int *next_vals = malloc(elems_left*sizeof(int));
            memcpy(next_vals, &vals[this_elems],elems_left*sizeof(int));
            
            int *child_stats = spawn_children(kids_left-1,elems_left,next_vals, ofp);
            
            write(ostream_child,child_stats,3*sizeof(int));

            
            free(next_vals);

            exit(1);

        }
    }else{
        
        int this_elems = max_size;
        //printf("vals are from %d, %d, to %d,\n",vals[0],vals[1],vals[2]);
        int *results = stats(0,this_elems,vals);
       

        int ostream_child;
        ostream_child = pipefds[1];
        
        close(pipefds[0]);

        //printf("I'm mr edge case %d, and I'm returning sum: %d, min: %d, max: %d,\n",(int)getpid(),results[0],results[1],results[2]);
        //exit(1);
        return results;
    }
    
}

int main(int argc, char *argv[]){

	FILE *ofp;
	char outputFilename[] = "part_b_outputSean2.txt";
	ofp = fopen(outputFilename,"w");

	int i;
	for (i = 1; i<=1; i++){
		fprintf (ofp, "For the list of size 10^%d:\n",i);
		//fprintf (ofp, "Hi I'm process %d and my parent is %d.\n", (int) getpid(), (int) getppid());
		
		int length = pow(10,i);
		int *vals = readFile(i);

        //now vals contains vals. everything after this can be split!

        //performing statistical operations on data
        clock_t begin = clock();
        int *answer = spawn_children(3,length,vals,ofp);
        clock_t end = clock();
        double time_spent = (double)(end-begin)/CLOCKS_PER_SEC;

        int sum = answer[0];
        int min = answer[1];
		int max = answer[2];

		fprintf(ofp, "Max=%d\n", max);
		fprintf(ofp, "Min=%d\n", min);	
		fprintf(ofp, "Sum=%d\n", sum);
		fprintf(ofp, "Time=%fsec\n\n",time_spent);

		free (vals);
	}
	fclose(ofp);
        


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

    return 0;
}