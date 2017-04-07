#include <stdio.h>		// for standard input/output
#include <math.h>		// for pow function
#include <stdbool.h> 	// required to use bool in C
#include <pthread.h>	// for threads
#include <stdlib.h>		// for atoi() function

// to compile: gcc -std=c99 hw2q1.c -pthread -lm

struct arg_struct {
	int arg1;		// integer n
	bool* arg2;		// boolean array of primes
};

/* Iterative function to reverse digits of num*/
// reference: http://www.geeksforgeeks.org/write-a-c-program-to-reverse-digits-of-a-number/
int reverse_digits(int num){
	// initialize the reverse number
    int rev_num = 0;
    while(num > 0){
    	// evertime we add a digit to rev_num, we move the previous digit left by multiplying by 10
        rev_num = rev_num*10 + num%10;
        num = num/10;
    }
    return rev_num;
}

void *rev_prime(void* param){
	// extracting the parameters and saving them in n and primes
	struct arg_struct *args = param;
	int n = args->arg1;
	bool* primes = args->arg2;

	for (int i = 10; i<=n; i++){
		int revi = reverse_digits(i);		// reverse of i

		// if the reverse of i is greater than n, we can't check it
		if (revi <= n){
			if (primes[i] && primes[revi]){
				// declare them as false after they are printed so they don't get printed twice
				primes[i] = false;
				primes[revi] = false;
				printf("%d and its reverse %d are primes.\n", i, revi);
			}
		}
	}

	pthread_exit(0);
}

void *all_primes(void *param){
	// extracting the parameters and saving them in n and primes
	struct arg_struct *args = param;
	int n = args->arg1;
	bool* primes = args->arg2;

	//initialize primes to true
	for (int i = 0; i<(n+1); i++){
		primes[i] = true;
	}

	int root = pow(n,0.5);	//rounded down to int
	for (int i = 2; i<=root; i++){
		if (primes[i]){	
			for (int j = i*i; j<=n; j+=i){
				primes[j] = false;
			}

		} 
	}
	printf("The prime numbers not exceeding %d are:", n);
	for (int i = 0; i <=n; i++){
		if (primes[i]){
			printf(" %d", i);
		}
	}

	printf("\n");
	pthread_exit(0);
}

int main(int argc, char *argv[]){
	// this program needs an extra argument for n, so argc = 2
	if (argc != 2) {
		fprintf(stderr,"usage: a.out <integer value>\n");
		return -1;
	}


	int x = atoi(argv[1]);	// the int entered by user
	// x must be a positive number
	if (x < 0) {
		fprintf(stderr,"%d must be >= 0\n",atoi(argv[1]));
		return -1;
	}

	pthread_t tid[2];		// the thread identifier, need 2 for 2 threads
	pthread_attr_t attr;	// set of thread attributes

	// get the default attributes
	pthread_attr_init(&attr);

	// create a bool array for all the ints from 0 to n
	// array must go to n+1 so that primes[n] can be used
	bool primes[x+1];	// declaring this here so that it can be used by all threads
    
    // using struct to pass n and the boolean array to both functions all_primes() and rev_prime()
    struct arg_struct func_args;
    func_args.arg1 = x;
    func_args.arg2 = primes;

	//create the thread
	pthread_create(&tid[0], &attr, all_primes, (void*) &func_args);
	
	// wait for the thread to exit
	pthread_join(tid[0],NULL);

	// create second thread
	pthread_create(&tid[1], &attr, rev_prime, (void*) &func_args);
	pthread_join(tid[1],NULL);

	return 0;
}
