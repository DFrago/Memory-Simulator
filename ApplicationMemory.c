#include "ApplicationMemory.h"
#include "Performance.h"
#include "DRAM.h"
#include "DRAM_Cache.h"
#include "VirtualMemory.h"

int readMemory(Address addr){
    beginMemoryAccess(addr,true);
    int result;
    result=vmRead(addr);
    endMemoryAccess(addr,result);
    return result;
}

void writeMemory(Address addr, int value){
    beginMemoryAccess(addr,false);
    vmWrite(addr,value);
    endMemoryAccess(addr,value);
}

void flushMemory(){
    flushCache();
}
