#ifndef APPLICATION_MEMORY_H
#define APPLICATION_MEMORY_H

#include "DRAM.h"

int readMemory(Address addr);
void writeMemory(Address addr, int value);
void flushMemory();

#endif
