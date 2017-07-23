#include <stdio.h>      // for standard input/output
#include <stdbool.h>    // required to use bool in C
#include <pthread.h>    // for threads
#include <stdlib.h>     // for atoi() function
#include <semaphore.h>  // for semaphores
#include <unistd.h>     // for sleep function


//pthread_mutex_t chopstick[5];

//cond var for each philosopher
pthread_cond_t phil[5];

bool chop_avail[5];         // 5 chopsticks, true if its available, false otherwise
pthread_mutex_t chops_lock; //mutex for changing chop_avail

//function to see if chops are available on left and right
void test_chops(int i){
    if (!chop_avail[i] || !chop_avail[(i+1) % 5]){  // if not both chopsticks are available
        pthread_cond_wait(&phil[i], &chops_lock);    // wait for them to both be available
        //pthread_mutex_lock(&chops_lock);
        test_chops(i);
    }
}


void mark_chops_unavailable(int i){
    //take the two chopsticks
    chop_avail[i] = false;
    chop_avail[(i+1) % 5] = false;

    pthread_cond_signal(&phil[(i+4)%5]);
    pthread_cond_signal(&phil[(i+1)%5]);
}


void *philosopher(void * param){
    int *temp = param;
    int i = *temp;

    int times_eaten = 0;
    printf("I'm philosopher %d, and I'm famished\n",i);
    sleep(1.5);
    
    do {

        pthread_mutex_lock(&chops_lock);

            //wait for chops to be available
            test_chops(i); 

            //if chops are available, take them           
            mark_chops_unavailable(i);

        pthread_mutex_unlock(&chops_lock);

        // take the two chopsticks
        //pthread_mutex_lock(&chopstick[i]);
        //pthread_mutex_lock(&chopstick[(i+1) % 5]);

        /* eat for awhile */
        printf("I'm philosopher %d, and I'm enjoying a tasty meal\n",i);

        sleep(1.5);

        // release the two chopsticks


        pthread_mutex_lock(&chops_lock);

            //puts down the chopsticks
            chop_avail[i] = true;
            chop_avail[(i+1) % 5] = true;
            
            pthread_cond_signal(&phil[(i+4)%5]);   // signal to neighbors that he's done
            pthread_cond_signal(&phil[(i+1)%5]);

            //THINKING
            printf("I'm philosopher %d, and I'm thinking about DJ Khaled's new single\n",i);
            sleep(1.5);

            //HUNGRY AGAIN
            printf("I'm philosopher %d, and I'm famished\n",i);
            sleep(1.5);
            pthread_cond_wait(&phil[i], &chops_lock); //wait for neighbors to eat before


        pthread_mutex_unlock(&chops_lock);

    } while(true);
}

int main(){
    pthread_t tid[5];
    pthread_attr_t attr;

    pthread_attr_init(&attr);

    for(int i = 0;i<5;i++){
        pthread_cond_init(&phil[i], NULL);
        chop_avail[i] = true;
    }

    pthread_mutex_init(&chops_lock, NULL);

    int starting_id[5];

    for(int i = 0;i<5;i++){
        starting_id[i] = i;
        pthread_create(&tid[i], &attr, philosopher, (void *) &starting_id[i]);
    }

    sleep(30);

    for(int i = 0;i<5;i++){
        pthread_cancel(tid[i]);
    }

    return 0;
}