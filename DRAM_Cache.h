#ifndef DRAM_CACHE_H
#define DRAM_CACHE_H

#include "DRAM.h"

// read/write a word of memory.
// This read/write employs a 2-Way Set Associative Write Deferred cache
// Each cache line is 32 bytes (see DRAM.h)
// Total cache size is specified below

#define CACHE_SIZE 256

void initCache();
int readWithCache(Address addr);
void writeWithCache(Address addr,int value);
void flushCache();

#endif
