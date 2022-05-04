// #include "idt.S"

/* void set_idt_desc(int vec, void* func)
 * Inputs: vec -- vector in IDT
 *         func -- function to be linked
 * Return Value: none
 * Function: fills IDT with assembly links */
void set_idt_desc(int vec, void* func);

/* void set_idt()
 * Inputs: none
 * Return Value: none
 * Function: fills IDT with assembly links */
void set_idt ();

// exceptions 
int32_t div_error();
int32_t debug();
int32_t non_maskable_int();
int32_t breakpoint();
int32_t overflow();
int32_t bound_range_exceeded();
int32_t invalid_opcode();
int32_t device_not_available();
int32_t double_fault();
int32_t coprocessor_segment_overrun();
int32_t invalid_tss();
int32_t segment_not_present();
int32_t stack_segment_fault();
int32_t gen_protection_fault();
int32_t page_fault();
int32_t assertion_failure();
int32_t floating_point_error();
int32_t alignment_check();
int32_t machine_check();
int32_t SIMD_floating_point();

// TODO - figure out system calls
int32_t halt (uint8_t status);
int32_t execute (const uint8_t* command);
int32_t read (int32_t fd, void* buf, int32_t nbytes);
int32_t write (int32_t fd, const void* buf, int32_t nbytes);
int32_t open (const uint8_t* filename);
int32_t close (int32_t fd);
int32_t getargs (uint8_t* buf, int32_t nbytes);
int32_t vidmap (uint8_t** screen_start);
int32_t gen_system_call();

// int handlers
// int32_t keyboard_handler();
// int32_t rtc_handler();

// int32_t set_handler (int32_t signum, void* handler_address);
// int32_t sigreturn (void);

