#include "lib.h"
#include "rtc.h"
#include "i8259.h"
#include "system_execute.h"
#include "schedule.h"

/* flag set when an interrupt occurs in the handler */
volatile uint32_t int_flag = 0;
volatile uint32_t process_flags[MAX_PID] = {0,0,0,0,0,0};
jump_table_t rtc_jump_s = {.open = (void*)rtc_open, .close = (void*)rtc_close, .read = (void*)rtc_read, .write = (void*)rtc_write};

/* void init_rtc()
 * Inputs: none
 * Return Value: none
 * Function: initializes the RTC */
void init_rtc(){
    cli();

    char prev_a, prev_b;

    rtc_count = 0;

    int i;
    for (i = 0; i < MAX_PID; i++) {
        rtc_scales[i] = 1;
    }

    // Intialize REG_B
    outb(REG_B, RTC_PORT);
    prev_b = inb(RTC_DATA);
    outb(REG_B, RTC_PORT);
    outb(prev_b | BIT6, RTC_DATA);  //turns on bit 6

    outb(REG_A, RTC_PORT);
    prev_a = inb(RTC_DATA);
    outb(REG_A, RTC_PORT);
    outb((prev_a & LOW4) | HIGHEST_RATE, RTC_DATA);  // initialize RTC to max rate
    enable_irq(RTC_IRQ);
    sti();
}

/* void rtc_handler()
 * Inputs: none
 * Return Value: none
 * Function: manages interrupts sent from RTC */
void rtc_handler() {
    cli();
    rtc_count++;

    outb(REG_C, RTC_PORT);
    inb(RTC_DATA);
    send_eoi(RTC_IRQ);

    // int_flag = 1;

    int i;
    for (i = 0; i < MAX_PID; i++) {
        if (rtc_count % rtc_scales[i] == 0) {
            process_flags[i] = 1;
        }
    }

    sti();
}

/* int32_t rtc_open(const uint8_t* filename)
 * Inputs: filename - name of file to open
 * Return Value: returns 0
 * Function: initializes RTC frequency to 2 Hz */
int32_t rtc_open(const uint8_t* filename) {
    int i;
    int fd = 0;
    pcb_t* pcb = pid_to_pcb(curr_pid);
    dentry_t f_directory_entry;


    for (i = 2; i < FILE_ARRAY_SIZE; ++i){
        // find the first open entry in the array
        if (pcb->file_array[i].flags != FILE_OPEN){
            fd = i;
            break;
        }
    }

    if (fd > 0){
        if (read_dentry_by_name(filename,&f_directory_entry) == -1){
            printf("did not read dentry correctly\n");
            return -1;
        }
        // now our temp directory entry has the value we need
        pcb->file_array[fd].file_op_table_ptr = &rtc_jump_s;
        pcb->file_array[fd].inode = 0;
        pcb->file_array[fd].fpos = 0;
        pcb->file_array[fd].flags = FILE_OPEN; 
        return fd;
    }

    cli();
    char prev_a; 
    outb(REG_A, RTC_PORT);
    prev_a = inb(RTC_DATA);
    outb(REG_A, RTC_PORT);
    outb((prev_a & LOW4) | RATE, RTC_DATA);  // set frequency to freq
    sti();
    return 0;
}

/* int32_t rtc_close(int32_t fd)
 * Inputs: fd - file descriptor to see which files are open
            current_pcb - current process block being run
 * Return Value: returns 0
 * Function: none */
int32_t rtc_close(int32_t fd) {

    pcb_t* pcb = pid_to_pcb(curr_pid);
    
    if (!pcb){
        printf("Passed in null ptr to pcb_close");
        return -1;
    }
    if ((fd >= FILE_ARRAY_SIZE) | (fd<2)) {
        printf("Passed in invalid fd to pcb_close");
        return -1;
    }

    pcb->file_array[fd].flags = FILE_NOT_OPEN;

    rtc_scales[curr_pid] = 1;

    return 0;
}

/* int32_t rtc_read(int32_t fd, const void* buf, int32_t nbytes)
 * Inputs: fd - file descriptor to see which files are open
           buf - buffer to copy information to
           nbytes - number of bytes to be copied
           current_pcb - current process block being run
 * Return Value: returns 0
 * Function: blocks RTC until OS generates a new interrupt */
int32_t rtc_read(int32_t fd, const void* buf, int32_t nbytes) {

    // printf("Entering rtc_read\n");

    // int_flag = 0;
    // while (int_flag == 0);
    process_flags[curr_pid] = 0;
    while (process_flags[curr_pid] == 0);
    return 0;
}

/* int32_t rtc_write(int32_t fd, void* buf, int32_t nbytes)
 * Inputs: fd - file descriptor to see which files are open
           buf - buffer to copy information to
           nbytes - number of bytes to be copied
           current_pcb - current process block being run
 * Return Value: returns 0 upon success, -1 upon failure
 * Function: changes RTC frequency */
int32_t rtc_write(int32_t fd, void* buf, int32_t nbytes) {

    if (buf == 0) {
        return -1;
    }

    int32_t freq = *(int32_t*) buf;   // cast input buffer to int

    // validate frequency -- nonzero, between 2 and 1024, power of 2
    if (freq == 0 || freq < LOWEST_FREQ || freq > HIGHEST_FREQ || (freq & (freq - 1)) != 0) {
        return -1;
    }

    
    // else {
    //     rtc_scales[curr_pid] = 1;
    // }

    // log2 calculator -- finds how many times can the frequency be divided by 2
    char count = 0;
    while (freq != 1) {
        freq /= 2;
        count++;
    }
    
    // uses frequency = 32768 >> (rate - 1) to calculate rate from freq
    char rate = HIGHEST_RATE - count + 1;  
    rate &= HIGHEST_RATE;
    
    // virtualize RTC
    if (rate != 0) {
        rtc_scales[curr_pid] = (int) (HIGHEST_RATE / rate);    
    } 

    cli();
    char prev_a; 
    outb(REG_A, RTC_PORT);
    prev_a = inb(RTC_DATA);
    outb(REG_A, RTC_PORT);
    outb((prev_a & LOW4) | LOWEST_FREQ, RTC_DATA);  // set frequency to freq
    sti();
    return 0;
}

