#include "pit.h"
#include "terminal.h"
#include "system_execute.h"
#include "lib.h"
#include "i8259.h"

/* void init_pit()
 * Inputs: none
 * Return Value: none
 * Function: initializes the PIT */
void pit_init () {
    cli();
    pit_count = 0;
    shells_done = 0;
    outb(PIT_MODE, PIT_CMD);
    outb((uint8_t)(FREQ_DIVIDER & 0xFF), CHANNEL_0); //low bits
    outb((uint8_t)(FREQ_DIVIDER >> 8), CHANNEL_0); //high bits
    enable_irq(PIT_IRQ);
    sti(); 
}

/* void pit_handler()
 * Inputs: none
 * Return Value: none
 * Function: manages interrupts sent by the pit, uses frequency divider to generate interrupts
 * at our desired frequency */    
void pit_handler() {
    pit_count++; // record that we received a pit_interrupt
    send_eoi(PIT_IRQ);
    schedule();
}
