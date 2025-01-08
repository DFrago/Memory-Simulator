#ifndef VIRTUAL_MEMORY_H
#define VIRTUAL_MEMORY_H

#include "DRAM.h"

// reading and writing of memory
int vmRead(Address addr);
void vmWrite(Address addr, int value);

// control of the virtual memory.  
// When disabled the memory addresses are interpreted as physical addresses - no translation occurs.
void vmDisable();
void vmEnable(Address pageTable);

#define PAGE_TABLE_ENTRIES 64
#define BYTES_PER_PTE 4        // each page table entry is 32-bits.
struct PageTableEntry{
    unsigned int physicalPageNumber:6;
    unsigned int valid:1;
    unsigned int filler:25;
};
typedef struct PageTableEntry PageTable[PAGE_TABLE_ENTRIES];

#endif
