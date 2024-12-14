#ifndef _STATS_H_
#define _STATS_H_

#include "scheduler.h" //need for task_t

/*
 * The prototypes of functions implemented in stats.cpp
 */
void compute_stats(task_t task_list[], u_int size, float cpu_usage);

#endif
