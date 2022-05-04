#define INTR_LINK(name, func)   \
.GLOBL name                 ;\
name:                       ;\
    pushal                  ;\
    pushfl ;\
    call func               ;\
    popfl ;\
    popal                   ;\
    iret                    

INTR_LINK(div_error_linkage, div_error)
INTR_LINK(debug_linkage, debug)
INTR_LINK(non_maskable_int_linkage, non_maskable_int)
INTR_LINK(breakpoint_linkage, breakpoint)
INTR_LINK(overflow_linkage, overflow)
INTR_LINK(bound_range_exceeded_linkage, bound_range_exceeded)
INTR_LINK(invalid_opcode_linkage, invalid_opcode)
INTR_LINK(device_not_available_linkage, device_not_available)
INTR_LINK(double_fault_linkage, double_fault)
INTR_LINK(coprocessor_segment_overrun_linkage, coprocessor_segment_overrun)
INTR_LINK(invalid_tss_linkage, invalid_tss)
INTR_LINK(segment_not_present_linkage, segment_not_present)
INTR_LINK(stack_segment_fault_linkage, stack_segment_fault)
INTR_LINK(gen_protection_fault_linkage, gen_protection_fault)
INTR_LINK(page_fault_linkage, page_fault)
INTR_LINK(assertion_failure_linkage, assertion_failure)
INTR_LINK(floating_point_error_linkage, floating_point_error)
INTR_LINK(alignment_check_linkage, alignment_check)
INTR_LINK(machine_check_linkage, machine_check)
INTR_LINK(SIMD_floating_point_linkage, SIMD_floating_point)
INTR_LINK(keyboard_handler_linkage,keyboard_handler)
INTR_LINK(rtc_handler_linkage,rtc_handler)
INTR_LINK(pit_handler_linkage,pit_handler)

