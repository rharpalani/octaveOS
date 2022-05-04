#pragma once
#include "i8259.h"
#include "lib.h"

#define KB_IRQ 1
#define KEYBOARD_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define KEYBUFSIZE 128
// #define KEYBUFSIZE 1024
#define ONE 0x02
#define ZERO 0x0b
#define Q 0x10
#define P 0x19
#define A 0x1e
#define L 0x26
#define Z 0x2c
#define M 0x32
#define TAB 0x0F
#define BACK 0x0E
#define ENTER 0x1C
#define SPACEBAR 0x39
#define SPACE_ASCII 32
#define F1 0x3B
#define F2 0x3C
#define F3 0x3D

#define EZERO 0xE0

#define PRESS_L_CTRL 0x1D
#define PRESS_R_CTRL 0x1D
#define RELEASE_L_CTRL 0x9D
#define RELEASE_R_CTRL 0x9D

#define PRESS_L_ALT 0x38
#define PRESS_R_ALT 0x38

#define RELEASE_L_ALT 0xB8
#define RELEASE_R_ALT 0xB8

#define PRESS_L_SHIFT 0x2A
#define PRESS_R_SHIFT 0x36
#define RELEASE_L_SHIFT 0xAA
#define RELEASE_R_SHIFT 0xB6
#define PRESS_CAPS_LOCK   0x3A
#define RELEASE_CAPS_LOCK   0xBA

#define DASH 0x0C
#define EQUAL 0x0D
#define LBRACK 0x1A
#define RBRACK 0x1B
#define SEMI 0x27
#define QUOTE 0x28
#define QUOTE_ASCII 39
#define TICK 0x29
#define BSLASH 0x2B
#define BSLASH_ASCII 92
#define COMMA 0x33
#define PERIOD 0x34
#define SLASH 0x35

#define CAPS 32

/* void init_keyboard()
 * Inputs: none
 * Return Value: none
 * Function: initializes the IRQ line for the keyboard */
void init_keyboard(); 

/* void keyboard_handler()
 * Inputs: none
 * Return Value: none
 * Function: manages interrupts sent from keyboard using switch table */
void keyboard_handler();

/* buffer to store characters entered on keyboard */
extern char keybuf[KEYBUFSIZE]; 

/* stores length of keyboard buffer */
extern unsigned int buflen;

/* keyboard flags */
extern volatile int enter_pressed;


