#include "schedule.h"
#include "pit.h"
#include "terminal.h"

/* void init_scheduler()
 * Inputs: none
 * Return Value: none
 * Function: initializes the scheduler by setting schedule_counter to 2 */ 
void init_scheduler() {
    schedule_counter = INIT_SCHEDULE_COUNT;
}

/* void schedule()
 * Inputs: none
 * Return Value: none
 * Function: implements scheudling by saving process context and switching between processes 
 * at specified time slices */    
void schedule() {

    register uint32_t saved_ebp asm("ebp");
    register uint32_t saved_esp asm("esp");
    
    if (pit_count<3 && !shells_done){ // to originally spawn a bunch of shells
        term_switch(pit_count);
        if (pit_count==2){
            shells_done = 1;
        }
        system_execute((uint8_t*)"shell");
    }
    if (pit_count==3){
        term_switch(0);
    }

    // do a bunch of scheduling stuff
    int next_pid = terminals[(++schedule_counter%3)].top_pid;
    setup_paging(next_pid);

    pcb_t* my_pcb = pid_to_pcb(curr_pid);
    pcb_t* next_pcb = pid_to_pcb(next_pid);

    my_pcb -> my_k_ebp = saved_ebp;
    my_pcb -> my_k_esp = saved_esp;

    tss.ss0 = KERNEL_DS; // to understand - tss only used on user-->kernel?
        // in that case, want to invalidate the kernel stack for that program?

    tss.esp0 = pid_to_ksb(next_pid);

    curr_pid = next_pid;

    flush_tlb();

    asm volatile(
        "movl %0, %%ebp ;"
        "movl %1, %%esp ;"
        :
        :"r"(next_pcb->my_k_ebp), "r"(next_pcb->my_k_esp)
            );

    return;
}
