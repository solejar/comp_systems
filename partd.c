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

int *iterativeSpawn(int numKids, int numElems, int* vals, int* data);
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

int * depthSpawn(int kids_left, int max_size, int *vals){

    int pipefds[2];


    if(pipe(pipefds)){
        perror("pipe");
        exit(1);
    }


    if (kids_left>0){

        pid_t child_pid;

        child_pid = fork();
        if(child_pid == -1){
            perror("fork()");
            exit(-1);
        }
        if(child_pid!=0){

   			printf ("Hi I'm process %d and my parent is %d.\n", (int) getpid(), (int) getppid());

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

int *iterativeSpawn(int numKids, int numElems, int* vals, int* data){
	for (int i = 0; i<numKids; i++){
			// spawn children needs start index, vals,
			int pipefds[2];
			if(pipe(pipefds)){
        		perror("pipe");
       	 		exit(1);
    		}

    		pid_t child_pid;

    		child_pid = fork();

        	if(child_pid == -1){
            	perror("fork()");
            	exit(-1);
        	}

        	if(child_pid == 0){
        	   	printf ("Hi I'm process %d and my parent is %d.\n", (int) getpid(), (int) getppid());
        	   	int ostream_child;
        	   	ostream_child = pipefds[1];	// write end of the pipe

        	   	close(pipefds[0]);	// close the read end of the pipe

        	   	int start = i*numElems;
        	   	int end = (i+1)*numElems - 1;

        

        	   	int *child_stats = stats(start,end,vals);

        	   	write(ostream_child,child_stats,3*sizeof(int));

        	   	exit(0);
	
        	}
        	else{
        		int istream_parent;
        		istream_parent = pipefds[0];
        		close(pipefds[1]);

        		int child_stats[3];
        		read(istream_parent, child_stats, 3*sizeof(int));
        		data = collate(data,child_stats);
        	}    		
		}

	return data;
}

int main(int argc, char *argv[]){

	FILE *ofp;
	char outputFilename[] = "part_c_output.txt";
	ofp = fopen(outputFilename,"w");

    printf ("Hi I'm parent with pid %d.\n", (int) getpid());	
	int i;
	for (i = 1; i<=1; i++){
		//fprintf (ofp, "For the list of size 10^%d:\n",i);
		//fprintf (ofp, "Hi I'm process %d and my parent is %d.\n", (int) getpid(), (int) getppid());
		
		int length = pow(10,i);
		int *vals = readFile(i);
		int numKids = 5;
		int numElems = length/(numKids);
		int *data = malloc(3*sizeof(int));
		int *deleteMe = data;
		data[0] = 0;		// temp sum
		data[1] = INT_MAX;	// temp min
		data[2] = INT_MIN;	// temp max
		clock_t begin = clock();
		
		data = depthSpawn(2, length,vals);

		free(deleteMe);
        //now vals contains vals. everything after this can be split!

        //performing statistical operations on data

        clock_t end = clock();
        double time_spent = (double)(end-begin)/CLOCKS_PER_SEC;

        int sum = data[0];
        int min = data[1];
		int max = data[2];

		// fprintf(ofp, "Max=%d\n", max);
		// fprintf(ofp, "Min=%d\n", min);	
		// fprintf(ofp, "Sum=%d\n", sum);
		// fprintf(ofp, "Time=%fsec\n\n",time_spent);

		printf("Max=%d\n", max);
		printf("Min=%d\n", min);	
		printf("Sum=%d\n", sum);
		printf("Time=%fsec\n\n",time_spent);

		free (vals);
	}
	fclose(ofp);
        



    return 0;
}