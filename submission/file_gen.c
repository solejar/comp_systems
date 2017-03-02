#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>

int main(){
	int i;
	for (i = 1; i <= 5; i++){
		FILE *ofp;
		char outputFilename[12];
		sprintf(outputFilename,"data_%d.txt",i);

		ofp = fopen(outputFilename, "w");
		if (ofp == NULL) {
			fprintf(stderr, "Can't open output file %s!\n", outputFilename);
			exit(1);
		}

		int num;
		srand(23); // random seed for random number
		int length = pow(10,i);	//size of file
		int j;
		for (j = 0; j<length; j++){
			num = rand()%10001;	// random int between 0 and 10000
			fprintf(ofp, "%d\n",num);
		}

		fclose(ofp);
	}

	return 0;
}