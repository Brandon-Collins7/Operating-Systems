/**
 * COMP 3500: Project 5 Scheduling
 * Created by Xiao Qin
 * Updated by Brandon Collins
 * 11/13/2024
 *
 */

#include <stdio.h>
#include "scheduler.h"

/*
 * Read data. To read task information from a file (i.e., file_descriptor) and to store data into task_list[].
 * The number of tasks stored in task_list[] is specified by size, which is an output.
 */
void
read_file(FILE *file_descriptor, task_t task_list[], u_int *size)
{
    /* read data from input file */
    /* Caveat: please place '&' before task_array[]; otherwise, your program will be 'core dumped' */
   int num = 0;
   while (fscanf(file_descriptor, "%u %u %u", &task_list[num].pid, &task_list[num].arrival_time, \
              &task_list[num].burst_time)!= EOF) {
        num++;
    }
    *size = num;

    //initialize remaining time as burst time 
    for (int i = 0; i < num; i++) {
        task_list[i].remaining_time = task_list[i].burst_time;
    }
}
