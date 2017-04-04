#include <stdio.h>		// for standard input/output
#include <stdbool.h> 	// required to use bool in C
#include <pthread.h>	// for threads
#include <stdlib.h>		// for atoi() function
#include <semaphore.h>	// for semaphores
#include <unistd.h>		// for sleep function

//gcc -std=c99 hw2q4.c -pthread

// need a semaphore for number of children
// semaphore for number of teachers

sem_t t;	// global semaphore for number of teachers
sem_t c;	// global semaphore for number of children
float R;	// child to teacher ratio required, current ratio must stay less than this
float curr_R;	// current child to teacher ratio
sem_t r_mutex;	// mutex lock for shared curr_R
sem_t t_mutex;	// mutex lock for teachers so no two teachers try to leave at the same time


int inf = 100000000;

void teacher_enter(){
	sem_post(&t);	// increment the number of teachers, if another was waiting to leave, it will be released
	printf("Teacher %u just arrived.\n", (int) pthread_self());
}

void teach(){
	printf("Teacher %u is teaching.\n", (int) pthread_self());
	sleep(10);	// teachers teach for 10 seconds
}

void teacher_exit(){

	int curr_teach;		// current number of teachers
	sem_getvalue(&t, &curr_teach);
	int curr_child;		// current number of children
	sem_getvalue(&c, &curr_child);

	float temp_R;

	if ( (curr_teach - 1) != 0 ){
		temp_R = curr_child/(curr_teach-1);
	}
	else{
		if (curr_child == 0){
			temp_R = 0;
		}
		else{
			temp_R = inf;
		}
	}

	// if when the teacher leaves the ratio will still be met, the teacher can leave freely
	if (temp_R <= R){
		sem_wait(&t); // decrement number of teachers
		printf("Teacher %u just left.\n", (int) pthread_self());

	}
	// else, teacher can't leave until another joins
	else {
		printf("Teacher %u wants to leave but can't. Will go back to work and then try again later.\n", (int) pthread_self());
		teach();	// teacher is teaching again
		teacher_exit();	// teacher attempts to leave again
	}
}

void go_home(){
	printf("%u is going home now.\n", (int) pthread_self());
	pthread_exit(0);	// person leaves
}

void child_enter(){
	sem_post(&c);	// increment the number of children
	printf("Child %u just arrived.\n", (int) pthread_self());
}

void child_exit(){
	sem_wait(&c); // decrement number of children
	printf("Child %u just left.\n", (int) pthread_self());
}

void learn(){
	printf("Child %u is learning.\n", (int) pthread_self());
	sleep(10);	// children learn for 10 seconds
}

void parent_enter(){
	printf("Parent %u just arrived.\n", (int) pthread_self());
}

void verify_compliance(){
	
	int curr_teach;		// current number of teachers
	sem_getvalue(&t, &curr_teach);
	int curr_child;		// current number of children
	sem_getvalue(&c, &curr_child);

	if ( curr_teach!= 0 ){
		curr_R = curr_child/curr_teach;
	}
	else{
		if (curr_child == 0){
			curr_R = 0;
		}
		else{
			curr_R == inf;
		}
	}
	
	sem_post(&r_mutex);	// unlock the resources

	if (curr_R <= R){	// if the compliance is verified
		printf("Parent %u has verified compliance!\n", (int) pthread_self());
	}
	else{
		printf("Parent %u is unsatisfied because the regulation is not met!\n", (int) pthread_self());
	}
}

void parent_exit(){
	printf("Parent %u just left.\n", (int) pthread_self());
}


void Teacher(){
	for (;;) {
		teacher_enter();
		// ... critical section ... //
		teach();
		sem_wait(&t_mutex);	// locking so that only one teacher can leave at a time
		teacher_exit();
		sem_post(&t_mutex);
		go_home();
	}
}

void Child(){
	for (;;) {
		child_enter();
		// ... critical section ... //
		learn();
		child_exit();
		go_home();
	}
}

void Parent(){ 
	for (;;) {
		parent_enter();
		// ... critical section ... //
		verify_compliance();
		parent_exit();
		go_home();
	}
}

int main(int argc, char ** argv){
  //first arg = # of teach
  //2nd arg = # of children
  sem_init(&c, 0, 0);		// initialize semaphore for children to 0
  sem_init(&t, 0, 0);		// initialize semaphore for teachers to 0
  sem_init(&r_mutex, 0, 1);		// initialize semaphore for teachers to 0
  sem_init(&t_mutex, 0, 1);		// initialize semaphore for teachers to 0
  
  R = 3.0;	// arbitrary number for ratio required
  
  int count_child = 9;
  int count_teacher = 5;
  int count_parent = 4;
  
  //threads instantiated here
  pthread_t child_tid[count_child];
  pthread_t teacher_tid[count_teacher];
  pthread_t parent_tid[count_parent];
  
  pthread_attr_t attr;  
  pthread_attr_init(&attr); // get the default attributes
  
  
  //creating all of the threads here
  for(int i = 0;i<count_teacher;i++){
    pthread_create(&teacher_tid[i],&attr,(void*) Teacher, NULL);
  }

  sleep(1);
  
  for(int i = 0;i<count_child;i++){
    pthread_create(&child_tid[i],&attr,(void *) Child, NULL);
  }
  
  for(int i = 0; i<count_parent;i++){
  	pthread_create(&parent_tid[i],&attr,(void*) Parent, NULL);
    sleep(3);
  }

  //joining the thread here
  for(int i = 0;i<count_teacher;i++){
    pthread_join(teacher_tid[i],NULL);
  }
  
  for(int i = 0;i<count_child;i++){
    pthread_join(child_tid[i], NULL);
  }
  
  for(int i = 0; i<count_parent;i++){
  	pthread_join(parent_tid[i], NULL);
  }
  

  sem_destroy(&c);
  sem_destroy(&t);
  sem_destroy(&r_mutex);
  sem_destroy(&t_mutex); 
  
  return 0;
}