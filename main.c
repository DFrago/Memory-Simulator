#include "Performance.h"
#include "ApplicationMemory.h"
#include "VirtualMemory.h"
#include "DRAM_Cache.h"
#include <stdio.h>

#define PAGE_SIZE 1024

void setupPageTable(){
    perfNote("\ncreating the page table\n");
    // build a page table that puts the lowest virtual page in the highest physical page
    int phyPage=48;
    for (int p=0;p<PAGE_TABLE_ENTRIES;++p) {
        --phyPage;
        int pte=0;
        if(phyPage>=0){
            pte=0x40|phyPage;   // set valid flag and page number
        }
        writeMemory(p*4,pte);
    }
    vmEnable(0);
}

void reportError(Address addr,int expected,int found){
    char msg[200];
    sprintf(msg,"ERROR: bad value @ %08x (%d)  value: %d  expected: %d\n",addr,addr,found,expected);
    printf("%s",msg);
    perfNote(msg);
}

// write the memory address at the start of pages 8 through 31 
// and then check that those values were properly recorded in memory.
// The tlb will not have any hits during this test.
void simplePageTest(){
    int value;
    for(int i=8*PAGE_SIZE;i<32*PAGE_SIZE;i+= PAGE_SIZE){
        writeMemory(i, i);
    }
    for(int i=8*PAGE_SIZE;i<32*PAGE_SIZE;i+=PAGE_SIZE){
        value=readMemory(i);
        if(value!=i){
            reportError(i, i, value);
        }
    }
}

// write and read to pages A, B, C, B, A
// this should result in 4 tlb misses:  A, B, C, A
// this should result in tlb hits for the read on each page and for the write on the second B access
void tlbTest(){
    int value;
    perfNote("starting tlb test\n");
    int A=10*PAGE_SIZE+0xA4;
    int B=20*PAGE_SIZE+0xB4;
    int C=30*PAGE_SIZE+0xC4;

    writeMemory(A,A);
    value=readMemory(A);
    if(value!=A){
        reportError(A,A,value);
    }

    writeMemory(B,B);
    value=readMemory(B);
    if(value!=B){
        reportError(B,B,value);
    }

    writeMemory(C,C);
    value=readMemory(C);
    if(value!=C){
        reportError(C,C,value);
    }

    writeMemory(B,B);
    value=readMemory(B);
    if(value!=B){
        reportError(B,B,value);
    }

    writeMemory(A,A);
    value=readMemory(A);
    if(value!=A){
        reportError(A,A,value);
    }
}

int main(int argc,char**argv)
{
    initCache();
    clearPerformanceCounters("memory_trace_3_main.txt");
    perfNote("Setting up page table\n");
    setupPageTable();
    perfNote("\nVirtual memory enabled\nstarting test with vm\n");
    simplePageTest();
    tlbTest();
    flushMemory();
    struct PerformanceCounters pc;
    getPerformanceCounters(&pc);
    printPerformanceInfo(&pc);


    printf("done\n");
}
