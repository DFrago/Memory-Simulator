#ifndef DRAM_H
#define DRAM_H

typedef unsigned int Address;
#define OFFSET_BITS  5
#define CACHE_LINE_SIZE   32      // (1 << OFFSET_BITS)
#define OFFSET_MASK (CACHE_LINE_SIZE - 1)

typedef unsigned char CacheLine[CACHE_LINE_SIZE];


// read/write a word of memory
int readDram(Address addr);
void writeDram(Address addr, int value);

// read/write a cache line
void readDramCacheLine(Address addr, CacheLine line);
void writeDramCacheLine(Address addr, CacheLine line);

#endif
