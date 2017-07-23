#include <stdio.h>
//for i/o

#include <stdlib.h>
#include <string.h> //for strcat, strcpy and for memcpy
#include <limits.h> //int_max, int_min
#include <math.h> //pow function, for file_size
#include <time.h> //for timing
#include <unistd.h> //interact w/ processes
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>

//need to link to math '-lm'
//gcc -std=c99 -o objectname filename.c -lm -D_POSIX_C_SOURCE=199309L

struct sigaction act;
union sigval value;
siginfo_t info;
sigset_t mask, mask2, mask3;

struct sigaction act2;

//this is the handler for when the kid takes too long
void alarm_handler(int signum){

    kill(getppid(),SIGRTMIN+6);

    printf("I'm %u and I'm going to sleep until i get killed.\n", getpid());
    sleep(10);
}

// this is the handler which all children use to attempt to kill the 
// misbehaving child
void mark_child_handler(int signum, siginfo_t *info, void *ptr){
    int val = info->si_value.sival_int;
    printf("Received signal %d\n", signum);
    printf("Signal originates from process %lu\n", (unsigned long)info->si_pid);
    printf("Misbehaving process is %d\n", val);
    printf("I am process %u\n", getpid());
    kill(val,SIGTERM);
}

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

//this file combines results of two arrays
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

//read in vals and put them in an array
int * readFile(int file, int data_size){

    const char * file_pref = "data_";
    const char * file_suff = ".txt";

    char inputFilename[24];
    sprintf(inputFilename,"data_%d.txt",file);

    //dynamically allocating array
    static int* output_vals;
    output_vals = malloc(data_size*sizeof(int));

    //this is opening file
    FILE *fp = NULL;
    fp = fopen(inputFilename,"r");

    //checking fopen() success
    if (fp == NULL){
        printf("\nWarning, fopen() failed!\n");
        int * error;
        *error = -1;
        return error;
    }   

    //reading in data 
    for(int i =0;i<data_size;i++){
        fscanf(fp,"%d", &output_vals[i]);
    }

    fclose(fp);

    return output_vals;
}

//spawn kids iteratively
int *iterative_spawn(int kids, int data_length, int * vals){
    FILE *output_file = NULL;
    const char * output_name = "part1b_output.txt";

    int *master_stats; 
    master_stats = malloc(3*sizeof(int));

    int *delete_me;
    delete_me = master_stats;

    //dummy vals to start out
    master_stats[0] = 0;
    master_stats[1] = INT_MAX;
    master_stats[2] = INT_MIN;

    //keep an array of children ids, so we can send messages to all of them.
    pid_t pid[kids];
 
    int elems_left = data_length;
    
    int start = 0;
    int end;

    //while the parent still has kids to spawn
    for(int i = 0;i<kids;i++){

        //find out how many to calculate
        double temp_elems = floor(elems_left/(kids-i));
        int this_elems = (int) temp_elems;

        end = start + this_elems;

        //figure out how many elems left to calc
        int temp = elems_left - this_elems;
        elems_left = temp;
        
        //keep track of pids in array
        pid[i] = fork();
        if(pid[i] ==0){//if it's the child

            //set an alarm for taking longer than 3 sec
            alarm(3);

            //arbitrarily set the 3rd child to misbehave, as proof of concept
            if (i == 3){
                sleep(4);
            }

            //sleep(2);

            //printf("this is parent, in the iterative func. id is %d, child is %d\n",(int) getpid(),(int) pid);
            output_file = fopen(output_name,"a");
            
            fprintf(output_file, "Hi, I'm process %d and my parent is %d.\n", (int)getpid(), (int)getppid());
            fclose(output_file);

            //calc my vals
            int * my_stats = stats(start,end,vals);

            //send my answers to my parent using sigqueue struct
            value.sival_int = my_stats[0];
            sigqueue(getppid(),SIGRTMIN,value);

            value.sival_int = my_stats[1];
            sigqueue(getppid(),SIGRTMIN+1,value);

            value.sival_int = my_stats[2];
            sigqueue(getppid(),SIGRTMIN+2,value);
            
            exit(1);
        }
        start = end;    
    }

    int all_child_stats[3][kids];

    // initialize stats to 0, max, and min so that collating doesn't mess up
    for (int k = 0; k<kids; k++){
        all_child_stats[0][k] = 0;
        all_child_stats[1][k] = INT_MAX;
        all_child_stats[2][k] = INT_MIN;
    }
    
    // counter to ensure to ensure parent receives all the data
    int sum_c = 0;
    int min_c = 0;
    int max_c = 0;

    // parent should 
    for (int i = 0;i<3*kids;i++){ 

        //get results from kid

        if (sigwaitinfo(&mask, &info) == -1) {
            perror("sigwaitinfo() failed");
        }else{
            int num = info.si_signo;
            int val = info.si_value.sival_int;
            if(num==SIGRTMIN){
              all_child_stats[0][sum_c] = val;
              sum_c++;
            }else if(num==SIGRTMIN+1){
              all_child_stats[1][min_c] = val;
              min_c++;
            }else if(num==SIGRTMIN+2){
              all_child_stats[2][max_c] = val;
              max_c++;
            }
            else if(num==SIGRTMIN+6){
                //printf("2. Here!!!, kids = %d\n",kids);
                i+=2;
                sum_c++;
                min_c++;
                max_c++;
                value.sival_int = info.si_pid;

                for(int j = 0; j < kids; j++){
                    sigqueue(pid[j],SIGUSR1, value);
                }
            }
        }
        //collate answers     

    }

    int dummyStatus;
    for (int i =0; i<kids; i++){
        wait(&dummyStatus);
    }

    // edit the collate to fix min, max, sum;

    if(sum_c == kids && min_c==kids &&max_c==kids){
        for(int i = 0;i<kids;i++){
            int child_stats[3];
            for(int j = 0;j<3;j++){
                child_stats[j] = all_child_stats[j][i];
            }
            int *temp  = collate(master_stats,child_stats);
            master_stats = temp;
        }
    }else{
            printf("Not all data from %d kids with %d sums, %d mins, %d maxs\n",kids, sum_c,min_c,max_c);
    }

free(delete_me);
return master_stats;
    
}

//spawn kids recursively
int * recursive_spawn(int kids_left, int max_kids, int max_size, int *vals){

    FILE *output_file = NULL;
    const char * output_name = "part1b_output.txt";

    //if still kids to be spawned
    if (kids_left>0){

        pid_t child_pid;

        child_pid = fork();
        if(child_pid == -1){
            perror("fork()");
            exit(-1);
        }

        if(child_pid!=0){//this is the parent
            output_file = fopen(output_name,"a");
            fprintf(output_file, "Hi, I'm process %d and my parent is %d.\n", (int)getpid(), (int)getppid());
            fclose(output_file);

            //figure out how many elems this one will handle
            int this_elems = (int) floor(max_size/(kids_left+1));

            //how many left to pass 
            int elems_left = max_size - this_elems;

            int *new_elems = malloc(this_elems*sizeof(int));

            //get results for parent
            for(int i= 0;i<this_elems;i++){
                new_elems[i] = vals[i];
            }
            int * test = stats(0,this_elems,vals);
            
            int kids_to_spawn;

            //edge case makes sure all kids have work to do
            if(this_elems<max_kids){
                kids_to_spawn = 1;
            }else{
                kids_to_spawn = max_kids+1;
            }

            //get my results, with my kids doing the work!
            int * parent_stats = iterative_spawn(kids_to_spawn,this_elems,new_elems);
            free(new_elems);

            //int all_to_send[3];   
            int next_elems = (int) floor(elems_left/(kids_left));            
            int child_stats[3];
            child_stats[0] = 0;
            child_stats[1] = INT_MAX;
            child_stats[2] = INT_MIN;            

            //read results from kid
            
            for (int i = 0; i<3; i++){
                if (sigwaitinfo(&mask2, &info) == -1) {
                    perror("sigwaitinfo() failed");
                }else{
                    int num = info.si_signo;
                    int val = info.si_value.sival_int;
                    if(num==SIGRTMIN+3){
                        child_stats[0] = val;
                    }else if(num==SIGRTMIN+4){
                        child_stats[1] = val;
                    }else if(num==SIGRTMIN+5){
                        child_stats[2] = val;
                    }
                    else if(num==SIGRTMIN+6){
                        printf("I'm HERE!!!");
                        i+=2;
                        value.sival_int = info.si_pid;
                        //sigqueue(0, SIGRTMIN+7, value);
                        sigqueue(child_pid,SIGUSR1,value);
                    }
                }
            }
            int dummyStatus;
            wait(&dummyStatus);
            int * results = collate(parent_stats, child_stats);

            return results;

        }else{//this is the child
            //sigprocmask(SIG_UNBLOCK, &mask3, NULL);
            alarm(3);

            //how many elems I am calculating?
            int this_elems = (int) floor(max_size/(kids_left+1));

            //size of the subarray
            int elems_left = max_size-this_elems;
            int next_elems = (int) floor(elems_left/(kids_left));

            //copies subarray to be passed to next function call
            int *next_vals = malloc(elems_left*sizeof(int));
            memcpy(next_vals, &vals[this_elems],elems_left*sizeof(int));
            
            //get results through recursive call
            int *child_stats = recursive_spawn(kids_left-1,max_kids,elems_left,next_vals);

            value.sival_int = child_stats[0];
            sigqueue(getppid(),SIGRTMIN+3,value);

            value.sival_int = child_stats[1];
            sigqueue(getppid(),SIGRTMIN+4,value);

            value.sival_int = child_stats[2];
            sigqueue(getppid(),SIGRTMIN+5,value);

            free(next_vals);
            exit(1);

        }
    }else{//this is the base case

        output_file = fopen(output_name,"a");
        fprintf(output_file, "Hi, I'm process %d and my parent is %d.\n", (int)getpid(), (int)getppid());
        fclose(output_file); 

        //figure out how many elems to operate on
        int this_elems = max_size;
        
        int *test = stats(0,this_elems,vals);

        int kids_to_spawn;

        //make this edge case so processes have at least one piece of data
        if(this_elems<max_kids){
            kids_to_spawn = 1;
        }else{
            kids_to_spawn = max_kids+1;
        }

        //get my results, by having my kids the work for me!
        int * parent_stats = iterative_spawn(kids_to_spawn,this_elems,vals);       

        //write results upstream to parent
        return parent_stats;
    }
    
}


int main(int argc, char *argv[]){
    //for mark_child_handler
    act2.sa_sigaction = mark_child_handler;
    act2.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &act2, NULL);

    // for alarm_handler for all the children who timeout
    signal(SIGALRM, alarm_handler);

    act.sa_flags = SA_SIGINFO;

    sigemptyset(&mask);
    sigemptyset(&mask2);
    sigaddset(&mask, SIGRTMIN);
    sigaddset(&mask, SIGRTMIN+1);
    sigaddset(&mask, SIGRTMIN+2);
    sigaddset(&mask, SIGRTMIN+6);   // for marking children
    //sigaddset(&mask, SIGRTMIN+7);   // for marking children

    sigaddset(&mask2, SIGRTMIN+3);
    sigaddset(&mask2, SIGRTMIN+4);
    sigaddset(&mask2, SIGRTMIN+5);
    sigaddset(&mask2, SIGRTMIN+6);
    // for marking children

    sigprocmask(SIG_BLOCK, &mask, NULL);
    sigprocmask(SIG_BLOCK,&mask2,NULL);

    sigemptyset(&act.sa_mask);

    sigaction(SIGRTMIN, &act, NULL);
    sigaction(SIGRTMIN+1, &act, NULL);
    sigaction(SIGRTMIN+2, &act, NULL);
    sigaction(SIGRTMIN+3, &act, NULL);
    sigaction(SIGRTMIN+4, &act, NULL);
    sigaction(SIGRTMIN+5, &act, NULL);
    sigaction(SIGRTMIN+6, &act, NULL);

    const char * output_name = "part1b_output.txt";

    FILE *output_file = NULL;
    output_file = fopen(output_name,"w");
    fclose(output_file);

    //open up files
    int kids = 3;
    for (int file_size = 1; file_size<=5; file_size++){

        FILE *ofp;
        char outputFilename[] = "part1b_output.txt";
        ofp = fopen(outputFilename,"a");

        fprintf (ofp, "For the list of size 10^%d:\n",file_size);
        fclose(ofp);

        int data_length = pow(10,file_size);

        int *vals = readFile(file_size,data_length);
        //now vals contains vals. everything after this can be split!

        //performing statistical operations on data
        clock_t begin = clock();
        int *answer = recursive_spawn(kids,kids,data_length,vals);
        clock_t end = clock();

        //get timings
        double time_spent = (double)(end-begin)/CLOCKS_PER_SEC;

        int sum = answer[0];
        int min = answer[1];
        int max = answer[2];

        //print answers
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