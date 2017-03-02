#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>

int main(){
	FILE *ofp;
	char outputFilename[] = "part_a_output.txt";
	ofp = fopen(outputFilename,"w");
	int i;
	for (i = 1; i<=5; i++){
		fprintf (ofp, "For the list of size 10^%d:\n",i);
		fprintf (ofp, "Hi I'm process %d and my parent is %d.\n", (int) getpid(), (int) getppid());

		FILE *ifp;

		char inputFilename[12];
		sprintf(inputFilename,"data_%d.txt",i);

		ifp = fopen(inputFilename, "r");
		if (ifp == NULL) {
			fprintf(stderr, "Can't open input file %s!\n", inputFilename);
			exit(1);
		}

		int num;
		int size = 0;
		while (fscanf(ifp, "%d", &num) != EOF){
			size = size+1;
		}

		int arr[size];

		rewind(ifp);	// go back to the beginning

		int sum = 0;	// initiating sum to 0
		int max = 0;	// initiating max to 0
		int min = INT_MAX;	// intiating min to INT_MAX
		int j;
		for (j = 0; j < size; j++){
			fscanf(ifp, "%d", &arr[j]);
			sum = sum + arr[j];
			if (arr[j] > max){
				max = arr[j];
			}
			if (arr[j] < min){
				min = arr[j];
			}
		}

		fprintf(ofp, "Max=%d\n", max);
		fprintf(ofp, "Min=%d\n", min);	
		fprintf(ofp, "Sum=%d\n\n", sum);

		fclose(ifp);
	}

	fclose(ofp);
	
	return 0;
}