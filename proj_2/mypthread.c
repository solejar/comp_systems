#include "ucontext.h"
#include "mypthread.h"
#include "stdio.h"
#include "time.h"
#include "stdlib.h"

//global variables
int cTID = 1; // current thread id
int totalThreads = 1; //total number of threads
int totalNodes = 0; //total number of nodes for circular linked list

mypthread_t* getThread(int tID) {
    threadNode = head;
    while (threadNode) {
        mypthread_t* thread = threadNode->node;
        //search through the list of running threads
        if (thread->tid == tID){
            //return the one that matches the current thread id
            return threadNode->node;
        }
        else {
            //otherwise continue looping until we find it
            threadNode = threadNode->next;
        }
    }
}

mypthread_t* nextActive(int tID) {
    //First search from head to find tID
    threadNode = head;
    //search for the specified thread in our list
    while (threadNode) {
        if (threadNode->node->tid == tID) {
            break; //break when we find the thread
        } 
        else {
            threadNode = threadNode->next;
        }
    }
    //as long as there exists a next node
    if (threadNode->next) {
        threadNode = threadNode->next;
        while (threadNode) {
            if (threadNode->node->state == ACTIVE) {
                return threadNode->node; //if its active return it
            } 
            else { //otherwise continue looping until we find the next active thread
                threadNode = threadNode->next;
            }
        }

    } 
    //if there are no more threads, exit 
    else {
        exit(0);
    }
}

//add data into the red robin
void addToRobin(mypthread_t* data) {
    //if the list is empty
    if (head == NULL) { 
        //make a new node
        threadNode = (struct node *) malloc(1 * sizeof(struct node));
        //set prev and next to null
        threadNode->prev = NULL;
        threadNode->next = NULL;
        //set the head and tail of the list to this new node
        head = threadNode;
        tail = threadNode;
        totalNodes++; //increment the total number of nodes
    } 
    else { //otherwise add data to the end of the existing list
        //make a new node
        threadNode = (struct node *) malloc(1 * sizeof(struct node));
        //set the tails next as this new node
        tail->next = threadNode;
        //set the prev of this new node equal to tail
        threadNode->prev = tail;
        //threadNode is now tail and becomes prev of head
        tail = threadNode;
        tail->next = head;
        head->prev = tail;
        totalNodes++; //increment the total number of nodes
    }
    threadNode->node = data; //finally set the actual data to this new node
}


int mypthread_create(mypthread_t *thread, const mypthread_attr_t *attr, void *(*start_routine)(void *), void *arg) {
    if (!totalNodes){ //if there are no threads we need to add the currently executing thread to the round robin
        mypthread_t* shellTh = (mypthread_t *) malloc(sizeof(mypthread_t)); //create a new thread object
        shellTh->tid = totalThreads++; //set the tid to totalThreads++ and increment it
        ucontext_t* c = (ucontext_t*) malloc(sizeof(ucontext_t)); //make a new context c
        shellTh->context = c; //set up the main thread's context
        shellTh->context->uc_stack.ss_sp = (char*) malloc(sizeof(char) * 4096); //declare its stack
        shellTh->context->uc_stack.ss_size = 4096; //declare its stack size explicitly
        shellTh->state = ACTIVE; //set the state to active
        addToRobin(shellTh); //add it to the round robin
    }
    //now once we made sure the main executing thread is in our round robin, we can add the specified thread to it
    ucontext_t* c = (ucontext_t*) malloc(sizeof(ucontext_t)); //create another context for the specified thread
    thread->context = c; //set its context
    getcontext(thread->context);//get the context of this thread
    (*thread).context->uc_stack.ss_sp = (char*) malloc(sizeof(char) * 4096); //set its stack up
    (*thread).context->uc_stack.ss_size = 4096; //set up its stack size
    (*thread).state = ACTIVE;//set its state to active
    thread->tid = totalThreads++;//set its thread id and increment total threads
    makecontext(thread->context, (void (*)()) start_routine, 1, arg); //make the context of this thread
    addToRobin(thread); //add it to the round robin
    return 0;
}

void mypthread_exit(void *retval) {
    //get the current thread from the list of threads
    mypthread_t* currTh = getThread(cTID);
    //set the state of the current thread to dead
    currTh->state = TERMINATED;
    //deallocate the memory allocated to the current threads context
    free(currTh->context);

    if (currTh->joined != 0){ //if the thread was joined somewhere else
        mypthread_t* joiner = getThread(currTh->joined); //get the thread that joined it
        //set the joiner thread to active state
        joiner->state = ACTIVE;
    }
    //find the next active thread from the list
    mypthread_t* next = nextActive(currTh->tid);

    cTID = next->tid; //set the current thread id to this new thread
    setcontext(next->context); //set the current context to this thread
    return;
}

int mypthread_yield(void) {
    //find the current thread based on its id
    mypthread_t* cTh = getThread(cTID);
    //find the next active thread
    mypthread_t* next = nextActive(cTh->tid);
    //set the current thread id
    cTID = next->tid;
    swapcontext(cTh->context, next->context); //yield to that thread
    return 0;
}

int mypthread_join(mypthread_t thread, void **retval) {
    //get the tid of the entered thread
    int switchToID = thread.tid;
    //find the current thread
    mypthread_t* curr = getThread(cTID);
    //find the entered thread
    mypthread_t* target = getThread(thread.tid);
    //if the state of the target isnt active, return 0
    if (target->state != ACTIVE) { //cant switch to dead thread
        return 0;
    } 
    //set the joined tid for the target to the current thread
    target->joined = cTID;
    //set the current thread equal to the new thread id
    cTID = switchToID;
    //set the state of the current thread to suspended
    curr->state = SUSPENDED;
    //swap their contexts
    swapcontext(curr->context, target->context);
    return 0;
}