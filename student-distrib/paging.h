#pragma once

#include "x86_desc.h"

#define VIDEO       0xB8000
#define VIDEO1      0xB9000
#define VIDEO2      0xBA000
#define VIDEO3      0xBB000
#define FOURKB         0x1000

#define KERNEL_LOCATION 0x00400
#define NUM_PAGES 1024
#define RIGHT_SHIFT 12
#define VMEM_MASK 0x000003ff

//initialize directory and table
page_directory_entry page_directory[NUM_PAGES]  __attribute__((aligned (NUM_PAGES * 4))); // the page directory
page_table_entry page_table_0[NUM_PAGES]  __attribute__((aligned (NUM_PAGES * 4))); // used to map the pages for the first 4MB, as 1024 pages size 4KB
page_table_entry page_table_1[NUM_PAGES]  __attribute__((aligned (NUM_PAGES * 4))); // used to map the pages for the next 4MB, as 1024 pages size 4KB

//functions to initialize paging
extern void enable_paging(void* page_directory_ptr);

/* init_paging()
*  DESCRIPTION: Initializes structures for paging, including page_directory,
                page_table, and assigning addresses.
   INPUTS: none
   OUTPUTS: none
   RETURN VALUE: none
*/
void init_paging();

/* flush_tlb()
*  DESCRIPTION: Flushes the TLB by writing to register CR3
   INPUTS: none
   OUTPUTS: none
   RETURN VALUE: none
*/
void flush_tlb();
