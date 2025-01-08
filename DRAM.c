#include "DRAM.h"
#include "Performance.h"
#include <memory.h>

unsigned char dram[49152];
// read/write a word of memory
int readDram(Address addr){
    int result;
    memcpy(&result,&dram[addr],4);
    perfDramRead(addr, result);
    return result;
}
void writeDram(Address addr, int value){
    memcpy(&dram[addr],&value,4);
    perfDramWrite(addr, value);
}

// read/write a cache line
void readDramCacheLine(Address addr, CacheLine line){
    memcpy(line,&dram[addr],32);
    perfDramCacheLineRead(addr, line);
}
void writeDramCacheLine(Address addr, CacheLine line){
    memcpy(&dram[addr],line,32);
    perfDramCacheLineWrite(addr, line);
}

