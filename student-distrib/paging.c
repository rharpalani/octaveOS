// #include "x86_desc.h"
#include "paging.h"

/* init_paging()
*  DESCRIPTION: Initializes structures for paging, including page_directory,
                page_table, and assigning addresses.
   INPUTS: none
   OUTPUTS: none
   RETURN VALUE: none
*/
void init_paging() {
    
    uint32_t vmem = (uint32_t)VIDEO; 

    int i;

    // setup default entries for both page directory and all entries in the one page table, init all to not present
    for (i = 0; i < NUM_PAGES; ++i) {
        page_directory[i].p = 0; // not present, the rest of the values don't matter
        page_directory[i].r_w = 1;
        page_directory[i].u_s = 0; 
        page_directory[i].pwt = 0; 
        page_directory[i].pcd = 1; 
        page_directory[i].a = 1; 
        page_directory[i].d = 0; 
        page_directory[i].ps = 1;
        page_directory[i].g = 0; 
        page_directory[i].avl = 0;
        page_directory[i].address_20 = 0;


        page_table_0[i].p = 0; // not present initially, rest shouldn't matter
        page_table_0[i].r_w = 1;
        page_table_0[i].u_s = 0; 
        page_table_0[i].pwt = 0; 
        page_table_0[i].pcd = 1; 
        page_table_0[i].a = 1; 
        page_table_0[i].d = 0; 
        page_table_0[i].pat = 0;
        page_table_0[i].g = 0; 
        page_table_0[i].avl = 0;
        page_table_0[i].address_20 = 0;

        page_table_1[i].p = 0; // not present initially, rest shouldn't matter
        page_table_1[i].r_w = 1;
        page_table_1[i].u_s = 0; 
        page_table_1[i].pwt = 0; 
        page_table_1[i].pcd = 1; 
        page_table_1[i].a = 1; 
        page_table_1[i].d = 0; 
        page_table_1[i].pat = 0;
        page_table_1[i].g = 0; 
        page_table_1[i].avl = 0;
        page_table_1[i].address_20 = 0;

    }

    // set first entry in page_directory to point to the page table
    page_directory[0].p = 1; // this page table is in memory atm
    page_directory[0].ps = 0; // we are pointing to page table
    page_directory[0].address_20 = ((uint32_t)(page_table_0) >> RIGHT_SHIFT); // pointer to the location of the page table


    /* VRAM SETUP */ 
    for (i= 0; i < 4; i++){
        page_table_0[((vmem >> RIGHT_SHIFT) & VMEM_MASK)+i].p = 1; // video memory is present
        page_table_0[((vmem >> RIGHT_SHIFT) & VMEM_MASK)+i].address_20 = ((vmem+i*FOURKB) >> RIGHT_SHIFT); // bc we are at same location in physical vs virtual memory
        page_table_0[((vmem >> RIGHT_SHIFT) & VMEM_MASK)+i].r_w = 1; // write & read
    }


    // kernel page setup
    page_directory[1].p = 1; // this page table is in memory atm
    page_directory[1].ps = 1; // we are pointing directly to large page
    page_directory[1].address_20 = KERNEL_LOCATION; // physical address offset, PAT, etc
    // bits 31-12 of the entry are 0x00400 - 0000 0000 0100 0000 0000
     // address with bit 22 = 1, all others 0
     // RSVD 0 , PAT = 0

    enable_paging(page_directory); 

}

/* flush_tlb()
*  DESCRIPTION: Flushes the TLB by writing to register CR3
   INPUTS: none
   OUTPUTS: none
   RETURN VALUE: none
*/
void flush_tlb(){
    asm volatile("pushl %eax \n"
                "movl %cr3, %eax \n"
                "movl %eax, %cr3 \n"
                "popl %eax \n"
            );
}
