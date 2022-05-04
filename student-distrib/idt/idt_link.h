#if !defined(IDT_H)
#define IDT_H

extern void div_error_linkage();
extern void debug_linkage();
extern void non_maskable_int_linkage();
extern void breakpoint_linkage();
extern void overflow_linkage();
extern void bound_range_exceeded_linkage();
extern void invalid_opcode_linkage();
extern void device_not_available_linkage();
extern void double_fault_linkage();
extern void coprocessor_segment_overrun_linkage();
extern void invalid_tss_linkage();
extern void segment_not_present_linkage();
extern void stack_segment_fault_linkage();
extern void gen_protection_fault_linkage();
extern void page_fault_linkage();
extern void assertion_failure_linkage();
extern void floating_point_error_linkage();
extern void alignment_check_linkage();
extern void machine_check_linkage();
extern void SIMD_floating_point_linkage();


extern void keyboard_handler_linkage();
extern void rtc_handler_linkage();
extern void pit_handler_linkage();

#endif /* IDT_H */
