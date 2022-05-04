#include "system_execute.h"
#include "x86_desc.h"
#include "paging.h"
#include "rtc.h"
#include "keyboard.h"
#include "terminal.h"
#include "filesys.h"

/* jump tables for stdio */
jump_table_t stdin = {.open = (void*)term_open, .close = (void*)term_close, .read = (void*)term_read, .write = (void*)term_open};
jump_table_t stdout = {.open = (void*)term_open, .close = (void*)term_close, .read = (void*)term_open, .write = (void*)term_write};
pcb_t kernel_pcb;

/* void init_processes
 * Inputs: none
 * Return Value: none
 * Function: initializes 6 processes, each with their own PCB */
void init_processes() {
    int i;
    for (i=0; i < MAX_PID; ++i){
        free_pid[i] = 1;
        memset(pid_to_pcb(i), 0, sizeof(pcb_t));
    }
}

/* void find_free_process
 * Inputs: none
 * Return Value: none
 * Function: keeps track of which processes are free for scheduling */
int find_free_process () {
    int i;
    for (i = 0; i < MAX_PID; ++i){
        if (free_pid[i]){
            return i;
        }
    }
    return -1;
}

/* int32_t system_execute(const uint8_t* command)
 * Inputs: command -- command to be executed
 * Return Value: none
 * Function: executes a given command, ensures an executable, creates a new process */
int32_t system_execute(const uint8_t* command) {

    // cli();
    register uint32_t saved_ebp asm("ebp");
    register uint32_t saved_esp asm("esp");


    // arg parsing, check exec
    uint8_t executable[EXEC_SIZE];
    int8_t cmd_holder[ARG_SIZE];
    int child_pid = find_free_process();
    if (child_pid == -1){
        printf("No free processes\n");
        return -1;
    }
    uint32_t entry_point;
    uint8_t file_name[FILE_NAME_SIZE];

    pid_to_term[child_pid] = curr_term; 

    // clearing filename and cmd_holder
    memset(file_name, 0, FILE_NAME_SIZE);
    memset(cmd_holder, 0, ARG_SIZE);

    // copying command into cmd_holder since command gets clobbered
    strncpy((int8_t*) cmd_holder, (int8_t*) command, ARG_SIZE);

    // parsing command into filename
    int cmd_idx = 0;
    int file_name_idx = 0;
    while(cmd_holder[cmd_idx] == ' ') {
        cmd_idx++;
    }
    
    while(cmd_holder[cmd_idx] != ' ' && cmd_holder[cmd_idx] != '\0') {
        file_name[file_name_idx] = cmd_holder[cmd_idx];

        cmd_idx++;
        file_name_idx++;
    }

    if (curr_pid > MAX_SHELLS) {
        if (0 == strncmp((int8_t*)file_name, (int8_t*)"shell", MAX_SHELLS)) {
            printf("max shells opened\n");
            return -1;
        }
    }

    const uint8_t* file_name_ptr = (uint8_t*) file_name;

    int file_descriptor = fopen(file_name_ptr);
    if (file_descriptor == -1){
        return -1;
    }
    int fdata = fread(file_descriptor, executable, EXEC_SIZE);
    
    //if file is not valid, return -1
    fclose(file_descriptor);

    if (fdata == -1) {
        return -1;
    }
    if (executable[0] != 0x7f || executable[1] != 0x45 || executable[2] != 0x4c || executable[3] != 0x46) {
        return -1;
    }

    // entry points
    entry_point = 0;
    entry_point |= executable[EP27];
    entry_point = entry_point << EIGHT_SHIFT;
    entry_point |= executable[EP26];
    entry_point = entry_point << EIGHT_SHIFT;
    entry_point |= executable[EP25];
    entry_point = entry_point << EIGHT_SHIFT;
    entry_point |= executable[EP24];

    terminals[curr_term].top_pid = child_pid;
    setup_paging(child_pid);
    
    // load file, double check if possible to coonver to type inode_t and access length
    uint32_t inode_number = pid_to_pcb(curr_pid)->file_array[file_descriptor].inode;
    inode_t* temp_inode = (inode_t*)(boot_block_location + inode_number + 1); //Double check to make sure that original pcb_holder is the pcb to keep all the info
    int32_t file_length = temp_inode->length;
    int32_t file_copy = read_data(inode_number, 0, (uint8_t*)(0x08048000), file_length);
    if (file_copy == -1) {
        return -1;
    }

    create_pcb(curr_pid, child_pid);
    initialize_stdio(pid_to_pcb(child_pid));

    // printf("PCB Created\n");
    pcb_t* current_pcb = pid_to_pcb(child_pid);
    pcb_t* parent_pcb = pid_to_pcb(curr_pid);

    // register uint32_t saved_ebp asm("ebp");
    current_pcb -> parent_ebp = saved_ebp;
    parent_pcb -> my_k_ebp = saved_ebp;
    // register uint32_t saved_esp asm("esp");
    current_pcb -> parent_esp = saved_esp;
    parent_pcb -> my_k_esp = saved_esp;

    // setting args
    memset(current_pcb -> args, 0, ARG_SIZE);    // clears previous args
    int arg_idx = 0;
    while(cmd_holder[cmd_idx] != '\0') {
        if (cmd_holder[cmd_idx] == ' ') {
            cmd_idx++;
        } else {
            current_pcb -> args[arg_idx] = cmd_holder[cmd_idx];
            cmd_idx++;
            arg_idx++;
        }
    }

    // prepare for context switch - set tss.ss0 and tss.esp0 to the correct values
    tss.esp0 = pid_to_ksb(child_pid);
    tss.ss0 = KERNEL_DS;
    curr_pid = child_pid;

    free_pid[curr_pid] = 0; // we are using this pid num
    
    asm volatile("push %0 \n"
                 "push %1 \n"
                 "pushfl \n"
                 "popl %%ebx \n"
                 "orl $0x0200, %%ebx \n"
                 "pushl %%ebx \n"
                 "push %2 \n"
                 "push %3 \n"
                 "iret"
                 :
                 :"r"(USER_DS), "r"(USER_STACK_BASE), "r"(USER_CS), "r"(entry_point)
                 :"memory", "cc", "ebx"
                );

    asm volatile ("ret_exec_label:");
    return 0; 
}

/* void setup_paging(int pid)
 * Inputs: pid -- process ID of function
 * Return Value: none
 * Function: sets up paging */
void setup_paging(int pid) {
    if (pid==-1){
        undo_user_paging(); 
        return;
    }
    uint32_t phys_addr = EIGHT_MB + (pid*FOUR_MB);
    // printf("Physical address is %d with pid %d\n",phys_addr,pid);
    page_directory[USER_PROGRAM_MEM_INDEX].u_s = 1;
    page_directory[USER_PROGRAM_MEM_INDEX].p = 1;
    page_directory[USER_PROGRAM_MEM_INDEX].ps = 1;
    page_directory[USER_PROGRAM_MEM_INDEX].address_20 = ((phys_addr & 0xffc00000) >> RIGHT_SHIFT); 
    // (phys_addr[31:22]) are top 10 bits, and the rest of address_20 should be 0, right shift by 12 to fit in the 20 bit space
    
            
    // phys_addr is the new physical address
    uint32_t vmem_phys_addr = (uint32_t)(terminals[pid_to_term[pid]].video_mem);
    uint32_t virtual_addr = (uint32_t)USER_VMEM_SYSCALL;

    // first set up the page_table entry
    int pte_ind = (virtual_addr >> RIGHT_SHIFT) & VMEM_MASK;
    
    page_table_1[pte_ind].p = 1; // video memory is present
    page_table_1[pte_ind].address_20 = (vmem_phys_addr >> RIGHT_SHIFT); 

    // flush TLB!!!!
    flush_tlb();
    
}

/* void create_pcb(int parent_pid, int child_pid)
 * Inputs: parent_pid -- process ID of parent function
 * Return Value: none
 * Function: creates the pcb */
void create_pcb(int parent_pid, int child_pid) {
    pcb_t* current_pcb = pid_to_pcb(child_pid);
    current_pcb->my_pid = child_pid;
    current_pcb->parent_pid = parent_pid;
    current_pcb->esp0 = tss.esp0;
    current_pcb->ss0 = tss.ss0;

}

/* void restore_tss(int child_pid)
 * Inputs: none
 * Return Value: none
 * Function: restores the TSS */
void restore_tss(int child_pid){
    pcb_t* current_pcb = pid_to_pcb(child_pid);
    tss.esp0 = current_pcb -> parent_esp;
    tss.ss0 = KERNEL_DS;

}

/* void undo_user_paging()
 * Inputs: none
 * Return Value: none
 * Function: undoes user paging */
void undo_user_paging() {
    page_directory[USER_PROGRAM_MEM_INDEX].p = 0;
    asm volatile("pushl %eax \n"
                "movl %cr3, %eax \n"
                "movl %eax, %cr3 \n"
                "popl %eax \n"
            );
}

/* uint32_t pid_to_ksb(int pid)
 * Inputs: pid -- process ID
 * Return Value: none
 * Function: kernel stack base */
uint32_t pid_to_ksb(int pid) {
    return (EIGHT_MB - 4 - EIGHT_KB*pid);
}

/* pcb_t* pid_to_pcb(int pid)
 * Inputs: pid -- process ID
 * Return Value: none
 * Function: gets the current pcb */
pcb_t* pid_to_pcb(int pid) {
    if (pid==-1){
        return &kernel_pcb;
    }
    return (pcb_t*)(EIGHT_MB - EIGHT_KB*(1+pid));
}

/* int32_t system_halt(uint8_t status)
 * Inputs: none
 * Return Value: none
 * Function: halts the system */
int32_t system_halt(uint8_t status){ 

    int copy_curr_pid = curr_pid;

    free_pid[curr_pid]=1; 
    int i;
    pcb_t* current_pcb = pid_to_pcb(curr_pid);

    // close all files in PCB so we don't get confused when we return
    for (i = 2; i < FILE_ARRAY_SIZE; i++){
        fclose(i);
    }

    // restore parent paging
    setup_paging(current_pcb->parent_pid);

    restore_tss(curr_pid);

    terminals[pid_to_term[curr_pid]].top_pid = current_pcb->parent_pid;
    curr_pid = current_pcb->parent_pid; // this process is done now

    if (copy_curr_pid<=2) {
        restart_shell = 1;
        while(1);
    }

    asm volatile(
        "movl %0, %%ebp ;"
        "movl %1, %%esp ;"
        "jmp ret_exec_label;"
        :
        :"r"(current_pcb->parent_ebp), "r"(current_pcb->parent_esp)
            );

    return 0;
}

/* void initialize_stdio(pcb_t* pcb_holder)
 * Inputs: pcb_holder -- holds the pcb
 * Return Value: none
 * Function: initializes standard in and standard out */
void initialize_stdio(pcb_t* pcb_holder) {

    pcb_holder->file_array[0].file_op_table_ptr = &stdin;
    pcb_holder->file_array[0].flags = FILE_OPEN;
    pcb_holder->file_array[0].fpos = 0;
    pcb_holder->file_array[0].inode = 0;

    pcb_holder->file_array[1].file_op_table_ptr = &stdout;
    pcb_holder->file_array[1].flags = FILE_OPEN;
    pcb_holder->file_array[1].fpos = 0;
    pcb_holder->file_array[1].inode = 0;
}

/* void undo_vmem_paging()
 * Inputs: none
 * Return Value: none
 * Function: undo video memory for paging */
void undo_vmem_paging(int32_t pid) {
    uint32_t virtual_addr = (uint32_t)USER_VMEM_SYSCALL;
    // int pde_ind = virtual_addr >> TOP_TEN_BITS;
    page_table_1[(virtual_addr >> RIGHT_SHIFT) & VMEM_MASK].p = 0;

    // flush the tlb
    flush_tlb();  
}

/* system_open()
*  DESCRIPTION: Uses jump tables to open file based on filename and filetype
*  INPUTS: filename - tells which file to open
*  OUTPUTS: none
*  RETURN VALUE: -1 if failed, 0 on success
*/
int32_t system_open(const uint8_t* filename) {
    dentry_t dentry;
    pcb_t* current_pcb;
    if (read_dentry_by_name(filename, &dentry) == -1) {     //if file isn't in the filesystem
        // printf("Failing read_dentry_by_name in system_open w filename %s\n",filename);
        return -1;
    }

    int32_t retval;

    // printf("Read dentry by name successfully with filename %s\n",filename);

    current_pcb = pid_to_pcb(curr_pid);     //set current pcb

    switch (dentry.filetype) {
        case(RTC_FILE):
            // printf("RTC_FILE");
            retval = rtc_open(filename);
            break;
            // return 0;
        case(DIR_FILE):
            // printf("DIR_FILE");
            retval =  dopen((uint8_t*)"ignored");
            break;
            // return 0;
        case(REG_FILE):
            // printf("REG_FILE");
            retval = fopen(filename);
            break;
            // return 0;
        default:
            // printf("default");
            retval = -1;    
    }

    // printf("retval at bottom of system_open is %d\n",retval);
    return retval;
}

/* system_close()
*  DESCRIPTION: Uses jump tables to close file based on fd
*  INPUTS: fd - index of file to close
*  OUTPUTS: none
*  RETURN VALUE: -1 if failed, 0 on success
*/
int32_t system_close(int32_t fd) {
    pcb_t* current_pcb;
    current_pcb = pid_to_pcb(curr_pid);

    if (fd > FILE_ARRAY_SIZE || fd < FILE_START_IDX || current_pcb->file_array[fd].flags == FILE_NOT_OPEN) {
        return -1;
    }

    jump_table_t* tmp_table = current_pcb->file_array[fd].file_op_table_ptr ;
    int32_t temp = tmp_table->close(fd); 
    return temp;

}

/* system_read()
*  DESCRIPTION: Uses jump tables to read file based on fd
*  INPUTS: fd - file descriptor to see which files are open
           buf - buffer to copy information to
           nbytes - number of bytes to be copied
*  OUTPUTS: none
*  RETURN VALUE: -1 if invalid, number if bytes read if success
*/
int32_t system_read(int32_t fd, void * buf, int32_t nbytes) {
    pcb_t* current_pcb;
    current_pcb = pid_to_pcb(curr_pid);

    if (fd > FILE_ARRAY_SIZE || fd < STDIN_IDX || fd == STDOUT_IDX || current_pcb->file_array[fd].flags == FILE_NOT_OPEN) {
        return -1;
    }


    // printf("In system read\n");
    jump_table_t* tmp_table = current_pcb->file_array[fd].file_op_table_ptr;
    if (fd>1) {
        // printf("Entering specific read inside of system_read\n");
    }
    int32_t temp = tmp_table->read(fd, buf, nbytes);
    return temp;
}

/* system_write()
*  DESCRIPTION: Uses jump tables to write to file based on fd
*  INPUTS: fd - file descriptor to see which files are open
           buf - buffer to copy information to
           nbytes - number of bytes to be copied
*  OUTPUTS: none
*  RETURN VALUE: -1 if invalid, number if bytes read if success
*/
int32_t system_write(int32_t fd, void * buf, int32_t nbytes) {
    pcb_t* current_pcb;
    current_pcb = pid_to_pcb(curr_pid);

    if (fd > FILE_ARRAY_SIZE || fd < STDIN_IDX || fd == STDIN_IDX || current_pcb->file_array[fd].flags == FILE_NOT_OPEN) {
        return -1;
    }

    // printf("In system write\n");

    jump_table_t* tmp_table = current_pcb->file_array[fd].file_op_table_ptr ;
    int32_t temp = tmp_table->write(fd, buf, nbytes); 
    return temp;
}

/* system_getargs(buf, nbytes)
*  DESCRIPTION: Reads the program's command line arguments into a user level buffer
*  INPUTS: buf - buffer to copy information to
           nbytes - number of bytes to be copied
*  OUTPUTS: none
*  RETURN VALUE: -1 if invalid, 0 if success
*/
int32_t system_getargs (uint8_t* buf, int32_t nbytes) {
    int i;
    pcb_t* current_pcb;
    current_pcb = pid_to_pcb(curr_pid);

    // copy args into buffer
    for (i = 0; i < nbytes; i++) {
        if (i < ARG_SIZE - 1) {
            buf[i] = current_pcb -> args[i];
        } else {
            break;
        }
    }

    return 0;
}

/* int32_t system_vidmap (uint8_t** screen_start)
 * Inputs: screen_start -- starting address in virtual memory
 * Return Value: none
 */
int32_t system_vidmap (uint8_t** screen_start) {

    termdata_t* termdata_ptr =  &(terminals[pid_to_term[curr_pid]]);

    // check to ensure screen_start is within the user memory
    if (((uint32_t) screen_start < USER_PROGRAM_MEM) || ((uint32_t)screen_start >= MB_132)) {
        return -1;
    }
    // do the remapping
    uint32_t phys_addr = (uint32_t)(termdata_ptr->video_mem);
    uint32_t virtual_addr = (uint32_t)USER_VMEM_SYSCALL;

    // first set up the page_table entry
    int pte_ind = (virtual_addr >> RIGHT_SHIFT) & VMEM_MASK;
    
    page_table_1[pte_ind].p = 1; // video memory is present
    page_table_1[pte_ind].address_20 = (phys_addr >> RIGHT_SHIFT); // bc we are at same location in physical vs virtual memory
    page_table_1[pte_ind].r_w = 1; // write & read
    page_table_1[pte_ind].u_s = 1; // user allowed


    // now map this in the page directory
    int pde_ind = (virtual_addr >> TOP_TEN_BITS);
    page_directory[pde_ind].p = 1; // this page table is in memory atm
    page_directory[pde_ind].ps = 0; // we are pointing to page table
    page_directory[pde_ind].address_20 = (((uint32_t)(page_table_1)) >> RIGHT_SHIFT); // pointer to the location of the page table
    page_directory[pde_ind].u_s = 1; // users can access this directory entry
    page_directory[pde_ind].r_w = 1; // write & read

    flush_tlb(); 

    *screen_start = (uint8_t*)USER_VMEM_SYSCALL;
    // last_screen_start = screen_start;
    
    return 0;
}

/* int32_t system_set_handler (int32_t signum, void* handler_address)
 * Inputs: none
 * Return Value: none
 * Function: sets the system handler */
int32_t system_set_handler (int32_t signum, void* handler_address) {
    return 0;
}

/* int32_t system_sigreturn (void)
 * Inputs: none
 * Return Value: none
 * Function: signal return */
int32_t system_sigreturn (void) {
    return 0;
}
