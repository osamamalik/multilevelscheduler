//
//  a2q2.c
//  
//
//  Created by Osama on 2019-03-09.
//

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <stdbool.h>

typedef int control;
#define LTS 1
#define STS 0

const int READY_QUEUE_CAPACITY = 5;
const int NEW_QUEUE_CAPACITY = 100;

int ready_queue_length = 0;
int new_queue_length = 0;

struct process {
    int PID;
    int burst_time;
    bool empty;
};

struct thread_parameters {
    struct process ready_queue[READY_QUEUE_CAPACITY];
    struct process new_queue[NEW_QUEUE_CAPACITY];
    control thread_control;
};

void printQueue(struct process queue[], int *length){
    int i = 0;
    for (i = 0; i < *length; i++){
        if (queue[i].empty == false){
            printf("PID: %d\n   Burst time: %d\n", queue[i].PID, queue[i].burst_time);
        }
    }
}

void enqueue(struct process queue[], int *length, struct process proc) {
    int i=0;
    for (i=0; i <= *length; i++) {
        if (queue[i].empty == true) {
            queue[i] = proc;
            break;      // unneccessary to look at remain queue spots
        }
    }
    
    *length = *length + 1;
}

struct process dequeue(struct process queue[], int *length) {
    struct process proc = queue[0];
    
    int i=0;
    for (i=0; i < *length-1; i++) {   // shift remaining processes to the left
        queue[i] = queue[i+1];
    }
    queue[i].empty = true;      // last process must be emptied
    *length = *length - 1;
    
    return proc;
}

void *long_term_scheduler(void *parameters) {
    printf("Entering Long Term Scheduler Thread\n");
    
    struct thread_parameters *t_params = (struct thread_parameters *) parameters;
    
    // run loop until the new queue is empty
    while (new_queue_length >= 1) {
        if (t_params->thread_control == LTS) {
            if (new_queue_length >= 1) {        // only do if new queue has processes
                int i = 0;
                for (i=0; i < READY_QUEUE_CAPACITY; i++) {      // take processes from new queue and place into ready queue
                    if (t_params->ready_queue[i].empty == true) {    // only place in ready queue if there is space
                        struct process proc = dequeue(t_params->new_queue, &new_queue_length);
                        printf("Dequeueing PID %d from New Queue: Burst Time Remaining = %d\n", proc.PID, proc.burst_time);
                        enqueue(t_params->ready_queue, &ready_queue_length, proc);
                        printf("Enqueueing PID %d into Ready Queue: Burst Time Remaining = %d\n\n", proc.PID, proc.burst_time);

                    }
                }
            }
            printf("Control passed to Short Term Scheduler\n\n");
            t_params->thread_control = STS;
        }
    }
    printf("New Queue is Empty\n\n");
    printf("Leaving Long Term Scheduler Thread\n\n");
    pthread_exit(NULL);
}

void *short_term_scheduler(void *parameters) {
    printf("Entering Short Term Scheduler\n");
    
    struct thread_parameters *t_params = (struct thread_parameters *) parameters;
    
    while (new_queue_length >= 1 || ready_queue_length >= 1) {
        if (t_params->thread_control == STS) {
            if (ready_queue_length >= 1) {      // only do if ready queue has processes
                int i = 0;
                for (i=0; i < READY_QUEUE_CAPACITY; i++) {
                    struct process proc = dequeue(t_params->ready_queue, &ready_queue_length);
                    printf("Dequeueing PID %d from Ready Queue: Burst Time Remaining = %d\n", proc.PID, proc.burst_time);
                    proc.burst_time = proc.burst_time - 2;
                    printf("Decreasing Burst Time of PID %d: Burst Time Remaining = %d\n\n", proc.PID, proc.burst_time);
                    if (proc.burst_time > 0) {      // only add to end of ready queue if process has time remaining
                        enqueue(t_params->ready_queue, &ready_queue_length, proc);
                        printf("Enqueueing PID %d into Ready Queue: Burst Time Remaining = %d\n\n", proc.PID, proc.burst_time);
                    }
                    else {
                        printf("Ending PID %d\n\n", proc.PID);
                        
                    }
                }
                
                if (new_queue_length < 1) {     // new queue is empty, so keep cycling through ready queue until it is empty
                    while (ready_queue_length >= 1) {
                        struct process proc = dequeue(t_params->ready_queue, &ready_queue_length);
                        printf("Dequeueing PID %d from Ready Queue: Burst Time Remaining = %d\n", proc.PID, proc.burst_time);
                        proc.burst_time = proc.burst_time - 2;
                        printf("Decreasing Burst Time of PID %d: Burst Time Remaining = %d\n", proc.PID, proc.burst_time);
                        if (proc.burst_time > 0) {
                            enqueue(t_params->ready_queue, &ready_queue_length, proc);
                            printf("Enqueueing PID %d into Ready Queue: Burst Time Remaining = %d\n\n", proc.PID, proc.burst_time);
                        }
                        else {
                            printf("Ending PID %d\n\n", proc.PID);

                        }
                    }
                    printf("Ready Queue is empty\n\n");
                }
                else {      // return control to long term scheduler
                    printf("Control passed to Long Term Scheduler\n\n");
                    t_params->thread_control = LTS;
                }
            }
        }
    }
    printf("Leaving Short Term Scheduler\n\n");
    pthread_exit(NULL);
}

int main() {
    srand(time(NULL));
    ready_queue_length = 0;
    new_queue_length = NEW_QUEUE_CAPACITY;
    struct thread_parameters t_params;
    
    int i;
    for (i=0; i < READY_QUEUE_CAPACITY; i++) {
        struct process proc;
        proc.PID = 0;
        proc.burst_time = 0;
        proc.empty = true;
        t_params.ready_queue[i] = proc;
    }
    
    for (i=0; i < NEW_QUEUE_CAPACITY; i++) {
        struct process proc;
        proc.PID = i+1;
        proc.burst_time = 1 + (rand()/((RAND_MAX/30) + 1));
        proc.empty = false;
        t_params.new_queue[i] = proc;
    }
    
    t_params.thread_control = LTS;
    
    pthread_t lts_thread;
    pthread_create(&lts_thread, NULL, long_term_scheduler, (void *) &t_params);
    pthread_t sts_thread;
    pthread_create(&sts_thread, NULL, short_term_scheduler, (void *) &t_params);
    pthread_join(lts_thread, NULL);
    pthread_join(sts_thread, NULL);
    
    return 0;
}
