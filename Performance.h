#ifndef PERFORMANCE_H
#define PERFORMANCE_H

#include "DRAM.h"
#include <stdbool.h>


struct PerformanceCounters {
    int dramReads;
    int dramWrites;
    int dramCacheLineReads;
    int dramCacheLineWrites;
    int cacheHits;
    int cacheMisses;
    int tlbHits;
    int tlbMisses;
    int dramCost;
};

void clearPerformanceCounters(char * filename);
void getPerformanceCounters(struct PerformanceCounters * pc);
void printPerformanceInfo(struct PerformanceCounters * pc);

// DRAM stats
void perfDramRead(Address addr,  int value);
void perfDramWrite(Address addr, int value);
void perfDramCacheLineRead(Address addr, CacheLine cl);
void perfDramCacheLineWrite(Address addr, CacheLine cl);

// cache stats
void perfCacheHit(Address addr, int set, int entry);
void perfCacheMiss(Address addr, int set, int entry, bool compulsory);
void perfCacheFlush();

// virtual memory stats
void perfStartAddressTranslation(Address virtAddr);
void perfEndAddressTranslation(Address phyAddr);
void perfTlbHit(int pageNumber);
void perfTlbMiss(int pageNumber);

void perfPageFault(int vpn);

void beginMemoryAccess(Address addr, bool isRead);
void endMemoryAccess(Address addr, int value);

void perfNote(const char * note);

void perfIndent();
void perfOutdent();

#endif
