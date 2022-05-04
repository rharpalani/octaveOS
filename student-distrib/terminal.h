#pragma once
#include "lib.h"
#include "keyboard.h"
#include "paging.h"
#include "i8259.h"
#include "system_execute.h"
#include "rtc.h"

#define MAX_TERM 3

int curr_term; 

/* multiple terminals struct */
typedef struct termdata_t { 
    char keybuf[KEYBUFSIZE];
    unsigned int buflen;
    volatile int enter_pressed;
    volatile int shift; // if 1, shift is pressed
    volatile int caps_lock; //if 1, caps_lock is on
    volatile int caps_lock_count;
    volatile int capital; //final flag to determine whether characters should be printed in capital
    volatile int ctrl; //if 1, ctrl is pressed
    volatile int alt;
    volatile int e_zero;
    int screen_x;
    int screen_y;
    char* video_mem;
    volatile int runon;
    volatile int top_pid;
    
} termdata_t;


/* global vars for term */
termdata_t terminals[MAX_TERM]; 
int pid_to_term[MAX_PID];

/* int32_t term_init()
 * Inputs: none
 * Return Value: returns 0
 * Function: none */
int32_t term_init();

/* int32_t term_open(const uint8_t* filename)
 * Inputs: none
 * Return Value: returns 0
 * Function: none */
int32_t term_open(const uint8_t* filename);

/* int32_t term_close(int32_t fd)
 * Inputs: none
 * Return Value: returns 0
 * Function: none */
int32_t term_close(int32_t fd);

/* int32_t term_read(int32_t fd, void* buf, int32_t nbytes)
 * Inputs: buf -- user buffer, copies to screen
 *         nbytes -- number of bytes to be copied
 * Return Value: returns the number of bytes copies
 * Function: none */
int32_t term_read(int32_t fd, void* buf, int32_t nbytes);

/* int32_t term_write(int32_t fd, const void* buf, int32_t nbytes)
 * Inputs: buf -- user buffer, copies to screen
 *         nbytes -- number of bytes to be copied
 * Return Value: returns the number of bytes copies
 * Function: none */
int32_t term_write(int32_t fd, const void* buf, int32_t nbytes);

/* void term_switch(int new_term)
 * Inputs: new_term -- terminal to be switched to
 * Return Value: none
 * Function: switches terminals by copying 'fake' video memory into 'real' video memory */
void term_switch(int new_term);
