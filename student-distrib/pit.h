#pragma once

#include "schedule.h"

#define PIT_IRQ 0
#define CHANNEL_0 0x40
#define PIT_CMD 0x43
#define PIT_MODE 0x34  //Mode 2
// #define PIT_INTERVAL 1193182 //1.19318 MHz
#define FREQ_DIVIDER 0x8000

/* PIT global vars */
unsigned int pit_count;
int shells_done;

/* void init_pit()
 * Inputs: none
 * Return Value: none
 * Function: initializes the PIT */
void pit_init();

/* void pit_handler()
 * Inputs: none
 * Return Value: none
 * Function: manages interrupts sent by the pit, uses frequency divider to generate interrupts
 * at our desired frequency */    
void pit_handler();
