#include "terminal.h"

/* int32_t term_init()
 * Inputs: none
 * Return Value: returns 0
 * Function: none */
int32_t term_init() {
    int i;
    for (i = 0; i < MAX_TERM; ++i){
        terminals[i].buflen = 0;
        terminals[i].enter_pressed =1;
        terminals[i].shift = 0; // if 1, shift is pressed
        terminals[i].caps_lock= 0; //if 1, caps_lock is on
        terminals[i].caps_lock_count= 0;
        terminals[i].capital= 0; //final flag to determine whether characters should be printed in capital
        terminals[i].ctrl= 0; //if 1, ctrl is pressed
        terminals[i].alt = 0; //If 1, alt is pressed
        terminals[i].e_zero = 0; //If 1, E0 present in scan code
        terminals[i].screen_x = 0;
        terminals[i].screen_y = 0;
        terminals[i].video_mem = (char *)(VIDEO + (i+1)*FOURKB);
        terminals[i].runon = 0;
        update_cursor();
    }
    terminals[0].video_mem = (char *)(VIDEO);
    return 0;
}

/* int32_t term_open(const uint8_t* filename)
 * Inputs: none
 * Return Value: returns 0
 * Function: none */
int32_t term_open(const uint8_t* filename) {
    // printf("In term_open");
    return 0;
}

/* int32_t term_close(int32_t fd)
 * Inputs: none
 * Return Value: returns 0
 * Function: none */
int32_t term_close(int32_t fd) {
    return 0;
}

/* int32_t term_read(int32_t fd, void* buf, int32_t nbytes)
 * Inputs: buf -- user buffer, copies from screen
 *         nbytes -- number of bytes to be copied
 * Return Value: returns the number of bytes copies
 * Function: copies contents of keyboard buffer into arg buffer */
int32_t term_read(int32_t fd, void* buf, int32_t nbytes) {
    // printf("In term_read");
    termdata_t* termdata_ptr =  &(terminals[pid_to_term[curr_pid]]);
    int32_t to_copy = KEYBUFSIZE;
    while(termdata_ptr->enter_pressed);   // do not copy unless enter key is pressed
    if (nbytes < to_copy) {     // cap nbytes to size of keyboard buffer
        to_copy = nbytes;
    }
    if (termdata_ptr->buflen < to_copy) {
        to_copy = termdata_ptr->buflen;
    }
    unsigned char* buf_cast = (unsigned char*) buf;
    int i;
    for (i = 0; i < to_copy; i++) {
        buf_cast[i] = termdata_ptr->keybuf[i];
    }
   
    termdata_ptr->enter_pressed = 1;
    memset(termdata_ptr->keybuf,0,KEYBUFSIZE);
    termdata_ptr->buflen = 0;
    return to_copy;
}

/* void term_switch(int new_term)
 * Inputs: new_term -- terminal to be switched to
 * Return Value: none
 * Function: switches terminals by copying 'fake' video memory into 'real' video memory */
void term_switch(int new_term) {
    // cli();
    if (curr_term == new_term){
        return;
    }

    // copy from real vmem into curr process temp vmem
    memset((char*)(VIDEO + FOURKB*(curr_term+1)) , 0, FOURKB);
    memcpy((char*)(VIDEO + FOURKB*(curr_term+1)) , (char*)VIDEO, FOURKB);
    
    // copy from new temp vmem into real vmem
    memset((char*)VIDEO, 0, FOURKB);
    memcpy((char*)VIDEO, (char*)(VIDEO + FOURKB*(new_term+1)), FOURKB);

    // curr terminal's vmem should point to its temp now
    terminals[curr_term].video_mem = (char*)(VIDEO + FOURKB*(curr_term+1));

    // new terms vmem should point to real now
    terminals[new_term].video_mem = (char*)(VIDEO);

    // system_vidmap(last_screen_start)


    // redo paging for system_vidmap compatability

    // phys_addr is the new physical address
    uint32_t phys_addr = (uint32_t)(terminals[pid_to_term[curr_pid]].video_mem);
    uint32_t virtual_addr = (uint32_t)USER_VMEM_SYSCALL;

    // first set up the page_table entry
    int pte_ind = (virtual_addr >> RIGHT_SHIFT) & VMEM_MASK;
    
    page_table_1[pte_ind].p = 1; // video memory is present
    page_table_1[pte_ind].address_20 = (phys_addr >> RIGHT_SHIFT); 

    flush_tlb(); 

    curr_term = new_term;
    update_cursor(); 
    // sti(); 
    
    return;
}

/* int32_t term_write(int32_t fd, const void* buf, int32_t nbytes)
 * Inputs: buf -- user buffer, copies to screen
 *         nbytes -- number of bytes to be copied
 * Return Value: returns the number of bytes copies
 * Function: prints contents of arg buffer to screen */
int32_t term_write(int32_t fd, const void* buf, int32_t nbytes) {
    char outchar;
    unsigned char* buf_cast = (unsigned char*) buf;
    int i;
    for (i = 0; i < nbytes; i++) {
        outchar = buf_cast[i];
        if (outchar) {
            putc(outchar);
        }
    }
    return nbytes;
}
