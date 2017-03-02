#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>

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

int * collate(int * array1, int *array2){
    int temp_sum = array1[0] + array2[0];
    int temp_min;
    int temp_max;

    static int output[3];    

    if(array1[2]<array2[2]){
        temp_min = array1[2];
    }else{
        temp_min = array2[2];
    }

    if(array1[1]>array2[1]){
        temp_max = array1[1];
    }else{
        temp_max = array2[1];
    }

    output[0] = temp_sum;
    output[2] = temp_min;
    output[1] = temp_max;

    return output;
}

int *stats(int start, int end, int *arr){

	int temp_sum = 0;		// initiating sum to 0
	int temp_max = INT_MIN;	// initiating max to 0
	int temp_min = INT_MAX;	// intiating min to INT_MAX
	for (int j = start; j <= end; j++){
		temp_sum = temp_sum + arr[j];
		if (arr[j] > temp_max){
			temp_max = arr[j];
		}
		if (arr[j] < temp_min){
			temp_min = arr[j];
		}
	}

	static int results[3];
	results[0] = temp_sum;
	results[1] = temp_max;
	results[2] = temp_min;

	return results;

}

int *spawnChild(int kids, int start, int end, int* arr, int* startData){
	int numElems = (end-start) + 1;	// 3 extra elements at the end for data
	int elms = numElems/(kids+1);
	int nextElms = numElems - elms;
	if (kids > 0){
		//fprintf (ofp, "Hi I'm process %d and my parent is %d.\n", (int) getpid(), (int) getppid());
		printf ("Hi I'm process %d and my parent is %d.\n", (int) getpid(), (int) getppid());
		int fds[2];

		int bufferElms = nextElms + 3;
		int *buffer;
		buffer = malloc(3*sizeof(int));

		pid_t pid;

		// Create a pipe. File descriptors for the two ends of the pipe are placed in fds.
		pipe(fds);

		// Fork a child process.
		pid = fork();

		if (pid == (pid_t) 0){
			// This is the child process. close our copy of the write end of the fild descriptor

			close(fds[1]);
			int n = read(fds[0], buffer, 3*sizeof(int));		// returns number of bytes read

			int *results;	

			// convert the read file descriptor to a FILE object, and read from it

			int *subarr = malloc(nextElms*sizeof(int));
			memcpy(subarr,&arr[elms],nextElms*sizeof(int));
			results = spawnChild(kids-1,start+elms,end, subarr, buffer);


			return results;

		}
		else {
			// This is the parent process
			// Close out copy of the read end of the file descriptor
			close (fds[0]);
			int *subarr = &arr[nextElms];
			int *results;
			results = stats(0,elms-1,arr);

			results = collate(results,startData);


			write(fds[1], results, 3*sizeof(int));
			// convert the write file descriptor to a FILE object, and write to it.
			exit(0);



			return results;

			//
		}
	}
	else{
		printf ("Hi I'm process %d and my parent is %d.\n", (int) getpid(), (int) getppid());
		int *results;
		results = stats(0,elms-1,arr);
		results = collate(results,startData);
		return results;
	}

}

int main(){
	FILE *ofp;
	char outputFilename[] = "part_b_output.txt";
	ofp = fopen(outputFilename,"w");
	int i;
	for (i = 1; i<=5; i++){
		fprintf (ofp, "For the list of size 10^%d:\n",i);
		fprintf (ofp, "Hi I'm process %d and my parent is %d.\n", (int) getpid(), (int) getppid());

		int length = pow(10,i);
		int *vals = readFile(i);

		static int tempData[3];
		tempData[0] = 0;
		tempData[1] = INT_MIN;
		tempData[2] = INT_MAX;

		int* data;

		data = spawnChild(4, 0,length-1,vals, tempData);

		int sum = data[0];
		int max = data[1];
		int min = data[2];
		fprintf(ofp, "Max=%d\n", max);
		fprintf(ofp, "Min=%d\n", min);	
		fprintf(ofp, "Sum=%d\n\n", sum);

	}

	fclose(ofp);
	
	return 0;
}