#include <stdio.h>		// for standard input/output
#include <stdbool.h> 	// required to use bool in C
#include <pthread.h>	// for threads
#include <stdlib.h>		// for atoi() function
#include <semaphore.h>	// for semaphores
#include <unistd.h>		// for sleep function

// to compile: gcc -std=c99 hw2q3.c -pthread
// to run: ./a.out <int for number of seats> <int for number of customers>

sem_t K;		// global semaphore variable for seats available -- counting semaphore
sem_t csdesk;	// global semaphore variable for customer service desk -- binary semaphore
char* bank_name = "Bank of America";		// assignment requests a shared variable, so all my customers will share the bank name

// assignment wants shared variable, but there really isnt much to share
// the below two variables will be shared just to varify that customers get serviced in the order that they sat down
int seat_number = 0;
int serv_number = 0;	

void make_transaction(){

	// decrement the service desk availability
	// if the value is 0, it will block until the desk is available
	sem_wait(&csdesk);

	// at this point, the customer is going to the cs desk. We can free up his seat
	sem_post(&K);		// make another seat available

	serv_number++;
	printf("Customer %u is being serviced!\n", (int) pthread_self());
	printf("Customer %u is the %d person to get serviced.\n",(int) pthread_self(), serv_number);
	sleep(3);		// 3 seconds to perform a transaction for this simulation

	// sem_post will increment the csdesk semaphore.
	// if csdesk is less than 0 after the increment, there is a thread blocking waiting for the desk
	// that blocking thread will be unblocked
	sem_post(&csdesk);
}

void take_a_walk(){
	printf("Customer %u is taking a walk.\n", (int) pthread_self());
	sleep(3);	// taking a walk will be simulated by a 3 second pause

}

void return_home(){
	printf("Customer %u is going home.\n", (int) pthread_self());
	pthread_exit(0);	// customer leaves, exit his thread
}


void bank_client(){
	int seats_available;
	printf("Customer %u arrived at %s.\n", (int) pthread_self(),bank_name);
	while(1) {
		sem_getvalue(&K, &seats_available);
		printf("Currently there are %d seats available for Customer %u.\n", seats_available, (int) pthread_self());

		if (seats_available > 0){	/* if seats available */
			sem_wait(&K);		// occupy a seat, this will decrement K
			printf("Customer %u got a seat!\n", (int) pthread_self());
			seat_number++;
			printf("Customer %u is the %d person to get a seat.\n", (int) pthread_self(), seat_number);
			make_transaction();	// attempt to make a transaction, will block until it can

			// -- question for group --
			/*
			We can make the seat available after the customer is done being serviced
			OR we can make the seat available as soon as the customer goes to the desk.
			Technically, the seat is available as soon as the customer is being serviced,
			so that is what I implemented. If we want to change that to the first option,
			we simply move "sem_post(&K)" from the make_transaction() function to here.
			*/

			break;
		}
		else{
			// attempt to do this with a wait instead
			take_a_walk();
		}
	}

	return_home();
}

int main(int argc, char* argv[]){
	// this program needs an extra argument for K, the number of seats
	// program also needs an extra argument for number of customers
	if (argc != 3) {
		fprintf(stderr,"usage: a.out <integer value> <integer value>\n");
		return -1;
	}


	int x = atoi(argv[1]);	// the int entered by user for K
	int custs = atoi(argv[2]);	// the int entered by user for customers
	// x and custs must be a positive number
	if (x < 0) {
		fprintf(stderr,"%d must be >= 0\n",atoi(argv[1]));
		return -1;
	}

	if (custs < 0) {
		fprintf(stderr,"%d must be >= 0\n",custs);
		return -1;
	}
	sem_init(&K, 0, x);		// initialize semaphore for seats available
	sem_init(&csdesk,0,1);	// intialize semaphore for the service desk
	pthread_t tid[custs];		// the thread identifier, need one for each cust
	pthread_attr_t attr;		// set of thread attributes

	// get the default attributes
	pthread_attr_init(&attr);

	for (int i = 0; i <custs; i++){
		// create the thread
		pthread_create(&tid[i], &attr, (void*) bank_client, NULL);
		// each time a thread is created, a customer arrives at the bank.
		// customers will arrive once every second.
		sleep(1);
	}

	for (int i =0; i< custs; i++){
		pthread_join(tid[i],NULL);
	}

	sem_destroy(&K);
	sem_destroy(&csdesk);
	return 0;
}
