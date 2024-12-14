/*
 * COMP 3500: Project 5 Scheduling
 * Brandon Collins
 * Version 1.0  11/12/2024
 *
 * This source code shows how to conduct separate compilation.
 *
 * scheduler.h: The header file of scheduler.c
 */
#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#define MAX_TASK_NUM 32

typedef unsigned int u_int;

typedef struct task_info {
    //set values
    u_int pid;
    u_int arrival_time;
    u_int burst_time;

    //found based on policy
    u_int start_time;
    u_int finish_time;
    u_int remaining_time; //updated as policy runs
} task_t;


/*
 * The prototypes of functions implemented in scheduler.c
 */

//each policy below returns CPU utilization

//first-come first-serve
float fcfsPolicy(task_t task_list[], u_int size);

//shortest remaining time first
float srtfPolicy(task_t task_list[], u_int size);

//round robin
float rrPolicy(task_t task_list[], u_int size, u_int quantum);


#endif
