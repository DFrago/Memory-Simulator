#include "Performance.h"
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

// current performance info
struct PerformanceCounters perf;
FILE * trace=NULL;

int indentAmt=0;

void doIndent(){
    for(int i = 0;i<indentAmt;++i){
        fprintf(trace,"\t");
    }
}

void perfIndent(){
    ++indentAmt;
}

void perfOutdent(){
    --indentAmt;
}

void clearPerformanceCounters(char * filename){
    memset(&perf,0,sizeof(perf));
    if(trace==NULL){
        trace=fopen(filename,"w");
    }
}

void getPerformanceCounters(struct PerformanceCounters * pc){
    perf.dramCost=perf.dramReads*24+
        perf.dramWrites*24+
        perf.dramCacheLineReads*36+
        perf.dramCacheLineWrites*36;
    memcpy(pc,&perf,sizeof(struct PerformanceCounters));
}

void printPerformanceInfo(struct PerformanceCounters * pc){
    time_t now;
    time(&now);
    fprintf(trace,"Run by %s at %s", getenv("USER"), ctime(&now));
    fprintf(trace,"Performance\n");
    fprintf(trace,"cost: %d\n", pc->dramCost);
    fprintf(trace,"DRAM reads: %d\n", pc->dramReads);
    fprintf(trace,"DRAM writes: %d\n", pc->dramWrites);
    fprintf(trace,"cache line reads: %d\n", pc->dramCacheLineReads);
    fprintf(trace,"cache line writes: %d\n", pc->dramCacheLineWrites);
    fprintf(trace,"cache hits: %d\n", pc->cacheHits);
    fprintf(trace,"cache misses: %d\n", pc->cacheMisses);
    fprintf(trace,"tlb hits: %d\n", pc->tlbHits);
    fprintf(trace,"tlb misses: %d\n", pc->tlbMisses);
}


void perfDramRead(Address addr,int value){
    ++perf.dramReads;
    doIndent();
    fprintf(trace,"dram read addr: %08x  value: %d\n", addr, value);
}

void perfDramWrite(Address addr,int value){
    ++perf.dramWrites;
    doIndent();
    fprintf(trace,"dram write addr: %08x  value: %d\n",addr,value);
}

void perfDramCacheLineRead(Address addr,CacheLine cl){
    ++perf.dramCacheLineReads;
    doIndent();
    fprintf(trace,"dram cache read addr: %08x  value:", addr);
    for(int i=0;i<CACHE_LINE_SIZE;++i){
        if ((i % 4) == 0) {
            fprintf(trace, " ");
        }
        fprintf(trace,"%02x",cl[i]);
    }
    fprintf(trace,"\n");
}

void perfDramCacheLineWrite(Address addr,CacheLine cl){
    ++perf.dramCacheLineWrites;
    doIndent();
    fprintf(trace,"dram cache write addr: %08x  value:", addr);
    for (int i=0;i<CACHE_LINE_SIZE;++i){
        if((i % 4)==0){
            fprintf(trace, " ");
        }
        fprintf(trace,"%02x",cl[i]);
    }
    fprintf(trace,"\n");
}

void perfCacheHit(Address addr,int set,int entry){
    ++perf.cacheHits;
    doIndent();
    fprintf(trace,"cache hit addr:%08x  set:%d  entry:%d\n",addr,set,entry);
}

void perfCacheMiss(Address addr,int set,int entry,bool compulsory){
    ++perf.cacheMisses;
    doIndent();
    fprintf(trace,"cache miss %s addr:%08x set:%d  entry:%d\n", 
            (compulsory ? "(empty)":""),addr,set,entry);
}

void perfCacheFlush(){
    doIndent();
    fprintf(trace,"flush\n");
}

void perfStartAddressTranslation(Address virtAddr){
    char msg[80];
    sprintf(msg,"Virtual address translation for %08x",virtAddr);
    perfNote(msg);
    perfIndent();
}

void perfEndAddressTranslation(Address phyAddr){
    char msg[80];
    sprintf(msg,"Physical address is %08x",phyAddr);
    perfNote(msg);
    perfOutdent();
}

void perfTlbHit(int pageNumber){
    ++perf.tlbHits;
    doIndent();
    fprintf(trace,"tlb hit for page %d\n",pageNumber);
}

void perfTlbMiss(int pageNumber){
    ++perf.tlbMisses;
    doIndent();
    fprintf(trace,"tlb miss for page %d\n",pageNumber);
}

void perfPageFault(int vpn){
    doIndent();
    fprintf(trace,"*** page fault ***   vpn=%d\n",vpn);
}

void beginMemoryAccess(Address addr, bool isRead){
    doIndent();
    fprintf(trace,"%s addr: %08x\n",(isRead ? "read":"write"),addr);
    perfIndent();
}

void endMemoryAccess(Address addr, int value){
    doIndent();
    fprintf(trace,"done addr: %08x value: %d\n",addr,value);
    perfOutdent();
}

void perfNote(const char * note){
    doIndent();
    fprintf(trace,"%s\n",note);
}
