#define ASM 1
#include "x86_desc.h"
.text
.GLOBL system_handler  
system_handler:
    #save flags
    pushfl

    #save registers
    pushl %ebx
    pushl %ecx
    pushl %edx
        
    pushl %ebp
    pushl %edi
    pushl %esi

    #push parameters
    pushl %edx
    pushl %ecx
    pushl %ebx

    #check if index is greater than 10
    cmpl $10, %eax
    jg invalid_idx

    #check if index is less than 0
    cmpl $0, %eax
    jle invalid_idx

    #jump table stuff
    call *system_table(, %eax, 4)
    jmp done

invalid_idx:
    movl $-1, %eax

done:
    #pop parameters
    addl $12, %esp

    #restore registers
    popl %esi
    popl %edi
    popl %ebp
    popl %edx
    popl %ecx
    popl %ebx

    #restore flags
    popfl

    iret

system_table:
    .long 0x00000000, system_halt, system_execute, system_read, system_write, system_open, system_close, system_getargs, system_vidmap, system_set_handler, system_sigreturn
