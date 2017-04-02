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

//this function combines the results of two arrays
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

//this function reads in vals
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

//this function recursively forks and splits work between children.
int * spawn_children(int kids_left, int max_size, int *vals, FILE* ofp){

    int pipefds[2];

    if(pipe(pipefds)){
        perror("pipe");
        exit(1);
    }

    //while there are still children left to spawn
    if (kids_left>0){

        pid_t child_pid;

        child_pid = fork();
        if(child_pid == -1){
            perror("fork()");
            exit(-1);
        }

        if(child_pid!=0){ //this is the parent

         	FILE *ofp;
			char outputFilename[] = "part_b_output.txt";
			ofp = fopen(outputFilename,"a");

   			//printf ("Hi I'm process %d and my parent is %d.\n", (int) getpid(), (int) getppid());
			fprintf (ofp, "Hi I'm process %d and my parent is %d.\n", (int) getpid(), (int) getppid());
            fclose(ofp);
			
            int istream_parent;
            istream_parent = pipefds[0];            

            close(pipefds[1]);

            //figure out how many elems this one will handle
            int this_elems = max_size/(kids_left+1);

            //how many left to pass to the kids
            int elems_left = max_size - this_elems;

            //get own results
            int * parent_stats = stats(0, this_elems,vals);
            
            int child_stats[3];

            //read in results from the kid
            read(istream_parent,child_stats,3*sizeof(int));

            int * results = collate(parent_stats, child_stats);

            return results;

        }else{//this is the child
            
            //printf("this is child. id is %d\n", (int) getpid());
            int ostream_child;
            ostream_child = pipefds[1];

            close(pipefds[0]);
            
            //this is how many elems this kid has to handle
            int this_elems = max_size/(kids_left+1);

            //size of the subarray
            int elems_left = max_size-this_elems;


            //copies subarray to be passed to next function call
            int *next_vals = malloc(elems_left*sizeof(int));
            memcpy(next_vals, &vals[this_elems],elems_left*sizeof(int));
            
            //get results from recursive function call
            int *child_stats = spawn_children(kids_left-1,elems_left,next_vals, ofp);
            
            //give those results back to the parent
            write(ostream_child,child_stats,3*sizeof(int));

            free(next_vals);

            exit(1);

        }
    }else{//this is the base case

        FILE *ofp;
        char outputFilename[] = "part_b_output.txt";
        ofp = fopen(outputFilename,"a");

        //printf ("Hi I'm process %d and my parent is %d.\n", (int) getpid(), (int) getppid());
        fprintf (ofp, "Hi I'm process %d and my parent is %d.\n", (int) getpid(), (int) getppid());
        fclose(ofp);

        //return results of subarray
        int this_elems = max_size;
        int *results = stats(0,this_elems,vals);

        return results;
    }
    
}

int main(int argc, char *argv[]){

	const char * output_name = "part_b_output.txt";

    FILE *output_file = NULL;
    output_file = fopen(output_name,"w");
    fclose(output_file);

    //open up all the files
	int i;
    int kids = 3;
	for (i = 1; i<=5; i++){
        FILE *ofp;
        char outputFilename[] = "part_b_output.txt";

        ofp = fopen(outputFilename,"a");
		fprintf (ofp, "For the list of size 10^%d:\n",i);
        fclose(ofp);

        //printf("For the list of size 10^%d:\n",i);
		
		int length = pow(10,i);
		int *vals = readFile(i);

        //now vals contains data. everything after this can be split!

        //performing statistical operations on data
        clock_t begin = clock();
        int *answer = spawn_children(kids,length,vals,ofp);
        clock_t end = clock();

        //get timings
        double time_spent = (double)(end-begin)/CLOCKS_PER_SEC;

        //print answers
        int sum = answer[0];
        int min = answer[1];
		int max = answer[2];

        /*printf("Max=%d\n", max);
        printf("Min=%d\n", min);  
        printf("Sum=%d\n", sum);
        printf("Time=%fsec\n\n",time_spent);*/

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