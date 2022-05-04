
.globl enable_paging
enable_paging:

    pushl %ebp
    movl %esp, %ebp


    mov 8(%esp), %eax
    mov %eax, %cr3

    mov %cr4, %eax
    or $0x00000010, %eax
    mov %eax, %cr4

    
    mov %cr0, %eax
    or $0x80000001, %eax
    mov %eax, %cr0



    leave
    ret

