/**
 * COMP 3500: Project 5 Scheduling
 * Created by Brandon Collins
 * 11/13/2024
 *
 */

#include <stdio.h>
#include "scheduler.h"

/*
 * Calculate and display stats of task list: 
 * avg waiting time, avg response time, and avg turnaround time
 * CPU utilization is already found in scheduler
 */
void
compute_stats(task_t task_list[], u_int size, float cpu_usage)
{
    float waitingTotal = 0;
    float responseTotal = 0;
    float turnaroundTotal = 0;

    for (int i = 0; i < size; i++){
        task_t task = task_list[i];

        //waiting = turnaround - cpu_time
        //response = start - arrival
        //turnaround = finish - arrival

        waitingTotal += task.finish_time - task.arrival_time - task.burst_time;
        responseTotal += task.start_time - task.arrival_time;
        turnaroundTotal += task.finish_time - task.arrival_time;

    }

    //print averages for each stat
    /* Sample output:
    ============================================================ 
    Average waiting time:    22.00 
    Average response time:   8.33 
    Average turnaround time: 28.83 
    Overall CPU usage:       100.00% 
    ============================================================
    */
    printf("============================================================\n");
    printf("Average waiting time:    %.2f\n", waitingTotal/size);
    printf("Average response time:   %.2f\n", responseTotal/size);
    printf("Average turnaround time: %.2f\n", turnaroundTotal/size);
    printf("Overall CPU usage:       %.2f%%\n", cpu_usage*100);
    printf("============================================================\n");
}

