#include "../x86_desc.h"
#include "idt.h"
#include "idt_link.h"
#include "../lib.h"
#include "../keyboard.h"
#include "../rtc.h"
#include "../system_execute_linkage.h"

/* void set_idt_desc(int vec, void* func)
 * Inputs: vec -- vector in IDT
 *         func -- function to be linked
 * Return Value: none
 * Function: fills IDT with assembly links */
void set_idt_desc(int vec, void* func) {
    idt_desc_t* ptr = idt;

    ptr += vec; 

    ptr -> seg_selector = KERNEL_CS;
    // dpl should be 3 for sys calls
    if (vec == 0x80) {
        ptr->dpl = 3; 
        ptr->reserved3 = 1; // for the trap gate
    }
    else {
        ptr -> dpl = 0; // for the kernel
        ptr->reserved3 = 0; // set to be an interrupt gate
    }
    ptr -> size = 1;
    if (!func) {
        ptr -> present = 0; // 0 for default (not present)
    }
    else {
        ptr -> present = 1; // 1 for actually setting
        ptr -> offset_15_00 = ((uint32_t)(func) & 0xFFFF);
        ptr -> offset_31_16 = ((uint32_t)(func) & 0xFFFF0000) >> 16;
    }

    // for both gate types, these 4 are the same
    ptr->reserved0 = 0;
    ptr->reserved1 = 1;
    ptr->reserved2 = 1;
    ptr->reserved4 = 0;

    return; 
}

/* void set_idt()
 * Inputs: none
 * Return Value: none
 * Function: fills IDT with assembly links */
void set_idt() {
    int i;

    // set them all to the default value
    for (i = 0; i < NUM_VEC; ++i){
        set_idt_desc(i,0);
    }

    // these are the exceptions
    set_idt_desc(0,div_error_linkage);
    set_idt_desc(1,debug_linkage);
    set_idt_desc(2,non_maskable_int_linkage);
    set_idt_desc(3,breakpoint_linkage);
    set_idt_desc(4,overflow_linkage);  
    set_idt_desc(5,bound_range_exceeded_linkage);
    set_idt_desc(6,invalid_opcode_linkage);
    set_idt_desc(7,device_not_available_linkage);    
    set_idt_desc(8,double_fault_linkage);
    set_idt_desc(9,coprocessor_segment_overrun_linkage);    
    set_idt_desc(0xa,invalid_tss_linkage);
    set_idt_desc(0xb,segment_not_present_linkage);
    set_idt_desc(0xc,stack_segment_fault_linkage);   
    set_idt_desc(0xd,gen_protection_fault_linkage);    
    set_idt_desc(0xe,page_fault_linkage);   
    set_idt_desc(0xf,assertion_failure_linkage); 
    set_idt_desc(0x10,floating_point_error_linkage);   
    set_idt_desc(0x11,alignment_check_linkage);
    set_idt_desc(0x12,machine_check_linkage);   
    set_idt_desc(0x13,SIMD_floating_point_linkage);

    // now the interrupt handlers
    set_idt_desc(0x20,pit_handler_linkage);
    set_idt_desc(0x21,keyboard_handler_linkage);
    set_idt_desc(0x28,rtc_handler_linkage);

    // general for system calls
    set_idt_desc(0x80, system_handler);

    return;
}

int32_t div_error(){
    printf("div0");
    while (1);
}
int32_t debug(){
    printf("debug");
    while (1);
}
int32_t non_maskable_int(){
    printf("NMI");
    while (1);
}
int32_t breakpoint(){
    printf("bkpt");
    while (1);
}
int32_t overflow(){
    printf("overflow");
    while (1);
}
int32_t bound_range_exceeded(){
    printf("bound_range_exceeded");
    while (1);
}
int32_t invalid_opcode(){
    printf("invalid_opcode");
    while (1);
}
int32_t device_not_available(){
    printf("device_not_available");
    while (1);
}
int32_t double_fault(){
    printf("double_fault");
    while (1);
}
int32_t coprocessor_segment_overrun(){
    printf("coprocessor_segment_overrun");
    while (1);
}
int32_t invalid_tss(){
    printf("invalid_tss");
    while (1);
}
int32_t segment_not_present(){
    printf("segment_not_present");
    while (1);
}
int32_t stack_segment_fault(){
    printf("stack_segment_fault");
    while (1);
}
int32_t gen_protection_fault(){
    printf("gen_protection_fault");
    while (1);
}
int32_t page_fault(){
    printf("page_fault");
    while (1);
}
int32_t assertion_failure(){
    printf("assertion_failure");
    while (1);
}
int32_t floating_point_error(){
    printf("floating_point_error");
    while (1);
}
int32_t alignment_check(){
    printf("alignment_check");
    while (1);
}
int32_t machine_check(){
    printf("machine_check");
    while (1);
}
int32_t SIMD_floating_point(){
    printf("SIMD_floating_point");
    while (1);
}
int32_t gen_system_call(){
    printf("gen_system_call");
    while (1);
}

