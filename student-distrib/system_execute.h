#pragma once
#include "filesys.h"
#include "types.h"
#include "terminal.h"
#include "paging.h"

#define EIGHT_MB 0x800000
// #define EIGHT_MB 0x007fffff
#define EIGHT_KB (EIGHT_MB >> 10)
#define FOUR_MB (EIGHT_MB >> 1)
#define FOUR_KB 4096
#define MB_132 0x8400000
#define USER_VMEM 0x07ffffff
#define USER_PROGRAM_MEM 0x08000000
#define USER_PROGRAM_MEM_INDEX (USER_PROGRAM_MEM >> 22)
#define USER_VMEM_INDEX (USER_VMEM >> 22)
#define USER_STACK_BASE 0x08400000 - 4
// #define USER_STACK_BASE 0x08400000
#define RTC_FILE 0
#define DIR_FILE 1
#define REG_FILE 2
#define TOP_TEN_BITS 22
#define USER_VMEM_SYSCALL MB_132
#define FILE_NAME_SIZE 32
#define ARG_SIZE 128
#define EXEC_SIZE 40
#define MAX_SHELLS 5

#define EIGHT_SHIFT 8
#define EP27 27
#define EP26 26
#define EP25 25
#define EP24 24

int curr_pid;
int restart_shell;
int free_pid[6];
// uint8_t** last_screen_start;

//define struct for PCB
typedef struct pcb_t { 
    fd_array_t file_array[8]; // 8 files in the file array, mp3 doc pg18
    int32_t my_pid;
    int32_t parent_pid;
    uint32_t parent_esp;
    uint32_t parent_ebp;
    uint32_t esp0;
    uint16_t ss0;
    uint8_t args[128];
    uint32_t my_k_esp;
    uint32_t my_k_ebp;
    // uint32_t my_u_esp;
    // uint32_t my_u_ebp;

    // uint32_t ebp;
    // uint16_t cs;
    // uint16_t ds;
    // uint16_t ss;
    // uint32_t eflags;
} pcb_t;

/* void setup_paging(int pid)
 * Inputs: pid -- process ID of function
 * Return Value: none
 * Function: sets up paging */
void setup_paging(int pid);

/* void undo_user_paging()
 * Inputs: none
 * Return Value: none
 * Function: undoes user paging */
void undo_user_paging();

/* pcb_t* pid_to_pcb(int pid)
 * Inputs: pid -- process ID
 * Return Value: none
 * Function: gets the current pcb */
pcb_t* pid_to_pcb(int pid);

/* int32_t system_halt(uint8_t status)
 * Inputs: none
 * Return Value: none
 * Function: halts the system */
int32_t system_halt(uint8_t status);

/* uint32_t pid_to_ksb(int pid)
 * Inputs: pid -- process ID
 * Return Value: none
 * Function: kernel stack base */
uint32_t pid_to_ksb(int pid);

/* int32_t system_execute(const uint8_t* command)
 * Inputs: command -- command to be executed
 * Return Value: none
 * Function: executes a given command, ensures an executable, creates a new process */
int32_t system_execute(const uint8_t* command);

/* void restore_tss(int child_pid)
 * Inputs: none
 * Return Value: none
 * Function: restores the TSS */
void restore_tss(int child_pid);

/* void create_pcb(int parent_pid, int child_pid)
 * Inputs: parent_pid -- process ID of parent function
 * Return Value: none
 * Function: creates the pcb */
void create_pcb(int parent_pid, int child_pid);

/* void initialize_stdio(pcb_t* pcb_holder)
 * Inputs: pcb_holder -- holds the pcb
 * Return Value: none
 * Function: initializes standard in and standard out */
void initialize_stdio(pcb_t* pcb_holder);

/* void undo_vmem_paging()
 * Inputs: none
 * Return Value: none
 * Function: undo video memory for paging */
void undo_vmem_paging(int32_t pid);

/* void init_processes
 * Inputs: none
 * Return Value: none
 * Function: initializes 6 processes, each with their own PCB */
void init_processes();

/* void find_free_process
 * Inputs: none
 * Return Value: none
 * Function: keeps track of which processes are free for scheduling */
int find_free_process();

/* system_open()
*  DESCRIPTION: Uses jump tables to open file based on filename and filetype
*  INPUTS: filename - tells which file to open
*  OUTPUTS: none
*  RETURN VALUE: -1 if failed, 0 on success
*/
int32_t system_open(const uint8_t* filename);

/* system_close()
*  DESCRIPTION: Uses jump tables to close file based on fd
*  INPUTS: fd - index of file to close
*  OUTPUTS: none
*  RETURN VALUE: -1 if failed, 0 on success
*/
int32_t system_close(int32_t fd);

/* system_read()
*  DESCRIPTION: Uses jump tables to read file based on fd
*  INPUTS: fd - file descriptor to see which files are open
           buf - buffer to copy information to
           nbytes - number of bytes to be copied
*  OUTPUTS: none
*  RETURN VALUE: -1 if invalid, number if bytes read if success
*/
int32_t system_read(int32_t fd, void * buf, int32_t nbytes);

/* system_write()
*  DESCRIPTION: Uses jump tables to write to file based on fd
*  INPUTS: fd - file descriptor to see which files are open
           buf - buffer to copy information to
           nbytes - number of bytes to be copied
*  OUTPUTS: none
*  RETURN VALUE: -1 if invalid, number if bytes read if success
*/
int32_t system_write(int32_t fd, void * buf, int32_t nbytes);

/* system_getargs(buf, nbytes)
*  DESCRIPTION: Reads the program's command line arguments into a user level buffer
*  INPUTS: buf - buffer to copy information to
           nbytes - number of bytes to be copied
*  OUTPUTS: none
*  RETURN VALUE: -1 if invalid, 0 if success
*/
int32_t system_getargs (uint8_t* buf, int32_t nbytes);

/* int32_t system_vidmap (uint8_t** screen_start)
 * Inputs: screen_start -- starting address in virtual memory
 * Return Value: none
 */
int32_t system_vidmap (uint8_t** screen_start);

/* int32_t system_set_handler (int32_t signum, void* handler_address)
 * Inputs: none
 * Return Value: none
 * Function: sets the system handler */
int32_t system_set_handler (int32_t signum, void* handler_address);

/* int32_t system_sigreturn (void)
 * Inputs: none
 * Return Value: none
 * Function: signal return */
int32_t system_sigreturn (void);
