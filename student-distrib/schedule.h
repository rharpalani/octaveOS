#pragma once

#define INIT_SCHEDULE_COUNT 2

/* global vars for scheduling */
int schedule_counter;

/* void init_scheduler()
 * Inputs: none
 * Return Value: none
 * Function: initializes the scheduler by setting schedule_counter to 2 */ 
void init_scheduler(); 

/* void schedule()
 * Inputs: none
 * Return Value: none
 * Function: implements scheudling by saving process context and switching between processes 
 * at specified time slices */  
void schedule();

