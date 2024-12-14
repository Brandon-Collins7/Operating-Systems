/*
 * COMP 3500: Project 5 Scheduling
 * Brandon Collins
 * Version 1.0  11/13/2024
 *
 * This source code shows how to conduct separate compilation.
 *
 * How to compile using Makefile?
 * $make
 * 
 * How to manually compile?
 * $g++ -c open.cpp
 * $g++ -c read.cpp
 * $g++ -c stats.cpp
 * $g++ open.o read.o stats.o scheduler.cpp -o scheduler
 *
 * How to run?
 * ./scheduler <file_name> [FCFS|RR|SRTF] [time_quantum]
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vector>
#include <queue> //for shortest remaining time first
// priority queue for shortest remaining time first

#include "scheduler.h"
//#include "print.h"
#include "open.h"
#include "read.h"
#include "stats.h"


/* First-Come First-Served */
/* processes run and complete in order that they arrive */
float fcfsPolicy(task_t task_list[], u_int size){
    int time = 0;
    int i = 0; //track which process to add to "queue"
    int timeNotUsed = 0;

    while(i < size){

        //process has arrived, so can do one time unit of work
        if (time >= task_list[i].arrival_time){

            if (task_list[i].remaining_time == task_list[i].burst_time){
                task_list[i].start_time = time;
            }

            //process is running
            printf("<time %u> process %u is running\n", time, task_list[i].pid);
            task_list[i].remaining_time--;
            time++;

            //process has finished
            //burst_time shouldn't be <= 0, so this should work
            if (task_list[i].remaining_time == 0){
                printf("<time %u> process %u is finished...\n", time, task_list[i].pid);
                task_list[i].finish_time = time;
                i++;
            }

        }

        //process had not arrived yet
        //CPU not utilized
        else{
            printf("<time %u> system is idle\n", time);
            time++;
            timeNotUsed++;
        }
    }

    printf("<time %u> All processes finished......\n", time);
    //return CPU utilization
    return (float)(time-timeNotUsed)/time;
}


/* Round Robin */
/* each process runs for a time quantum, then the next process in line runs */
float rrPolicy(task_t task_list[], u_int size, u_int quantum){
    int time = 0;
    int i = 0; //track which process to add to queue
    int timeNotUsed = 0;
    int finished = 0;
    int timeInQuantum = 0; //track how long a process has been running in quantum
    std::queue<int> q; //need queue to rotate processes

    //while there are still processes not finished
    while (finished < size){

        //add processes to queue that have arrived
        while (i < size && time >= task_list[i].arrival_time){
            q.push(i);
            i++;
        }

        //no processes in queue
        if (q.empty()){
            printf("<time %u> system is idle\n", time);
            time++;
            timeNotUsed++;
        }

        else{
            //index of current process
            int current = q.front();

            //process has not finished, but quantum has been reached
            //move to back of queue
            //in this order to meet rule:
            /* In case that both a new arrival and a running 
            process that completes its time slice must be replaced to the 
            end of the ready queue at the same time, the newly arrived 
            process must be inserted ahead of the process that gives up 
            the CPU. */
            if (timeInQuantum == quantum){
                q.pop();
                q.push(current);
                timeInQuantum = 0;
                current = q.front();
            }

            //process has begun, so mark start time
            if (task_list[current].remaining_time == task_list[current].burst_time){
                task_list[current].start_time = time;
            }

            //process is running
            printf("<time %u> process %u is running\n", time, task_list[current].pid);
            task_list[current].remaining_time--;
            time++;
            timeInQuantum++;

            //process has finished, so mark finish time and remove from queue
            //reset timeInQuantum
            if (task_list[current].remaining_time == 0){
                printf("<time %u> process %u is finished...\n", time, task_list[current].pid);
                task_list[current].finish_time = time;
                finished++;
                q.pop();
                timeInQuantum = 0;
            }

            
        }
    }

    printf("<time %u> All processes finished......\n", time);
    //return CPU utilization
    return (float)(time-timeNotUsed)/time;
}


/* Shortest Remaining Time First */
/* run process with shortest remaining time at each time unit */
float srtf(task_t task_list[], u_int size){
    int time = 0;
    int i = 0; //track which process to add to heap/queue
    int timeNotUsed = 0;
    int finished = 0;

    auto comp = [&task_list](int i1, int i2) { return task_list[i1].remaining_time > task_list[i2].remaining_time; };
    
    //use heap / priority queue to store processes with shortest remaining time
    std::priority_queue<int, std::vector<int>, decltype(comp)> heap(comp);

    while (finished < size){

        //add processes to heap that have arrived
        while (i < size && time >= task_list[i].arrival_time){
            heap.push(i);
            i++;
        }

        //no processes in heap
        if (heap.empty()){
            printf("<time %u> system is idle\n", time);
            time++;
            timeNotUsed++;
        }

        else{
            //index of current process
            int current = heap.top();

            //process has begun, so mark start time
            if (task_list[current].remaining_time == task_list[current].burst_time){
                task_list[current].start_time = time;
            }

            //process is running
            printf("<time %u> process %u is running\n", time, task_list[current].pid);
            task_list[current].remaining_time--;
            time++;

            //process has finished, so mark finish time and remove from heap
            if (task_list[current].remaining_time == 0){
                printf("<time %u> process %u is finished...\n", time, task_list[current].pid);
                task_list[current].finish_time = time;
                finished++;
                heap.pop();
            }
        }
    }

    printf("<time %u> All processes finished......\n", time);
    //return CPU utilization
    return (float)(time-timeNotUsed)/time;
}



int main( int argc, char *argv[] )  {
    char *file_name; /* file name from the commandline */
    FILE *fp; /* file descriptor */
    task_t task_array[MAX_TASK_NUM];

    int error_code;
    u_int i;
    u_int count;
    float cpuUsage;

    //command line parser
    /*
     * ./scheduler <file_name> [FCFS|RR|SRTF] [quantum]
    */

    //error handling
    if (argc < 3 || argc > 4) {
        printf("Usage: scheduler task_list_file [FCFS|RR|SRTF] [time_quantum]\n");
        return EXIT_FAILURE; 
    }
    if (argc == 3 && strcmp(argv[2], "RR") == 0){
        printf("Usage: input [time_quantum]\n");
        return EXIT_FAILURE;
    }
    if (strcmp(argv[2], "FCFS") != 0 && strcmp(argv[2], "RR") != 0 && strcmp(argv[2], "SRTF") != 0){
        printf("Usage: input valid schedule as shown by [FCFS|RR|SRTF]\n");
        return EXIT_FAILURE;
    }

    //make sure can open file
    error_code = open_file(argv[1], &fp);
    if (error_code == 1)
        return EXIT_FAILURE;

    printf("Scheduling Policy: %s\n", argv[2]);

    read_file(fp, task_array, &count);
    

    printf("There are %d tasks loaded from \"%s\". Press any key to continue ...", count, argv[1]);
    getchar();
    printf("============================================================\n");
    
    //choose scheduling policy
    if (strcmp(argv[2], "FCFS") == 0){
        cpuUsage = fcfsPolicy(task_array, count);
    }
    else if (strcmp(argv[2], "RR") == 0){
        cpuUsage = rrPolicy(task_array, count, atoi(argv[3]));
    }
    else if (strcmp(argv[2], "SRTF") == 0){
        cpuUsage = srtf(task_array, count);
    }
    else{ //this shouldn't happen since this is checked earlier
        printf("Invalid scheduling policy\n");
        return EXIT_FAILURE;
    }

    compute_stats(task_array, count, cpuUsage);

    fclose(fp); //close file
    return EXIT_SUCCESS;
}
