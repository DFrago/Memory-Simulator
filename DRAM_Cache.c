#include "DRAM.h"
#include "DRAM_Cache.h"
#include "Performance.h"
#include <stdbool.h>
#include <string.h>

typedef struct Line{
    bool valid;
    unsigned short tag;
    bool dirty;
    int timestamp;
    CacheLine data;
} Line;

typedef struct TranslatedAddress{
    short tag;
    unsigned char setIndex;
    unsigned char offset;
} TranslatedAddress;

Line cache[4][2];
static int time = 0;

void initCache(){
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 2; j++){
            cache[i][j].valid = false;
            cache[i][j].timestamp = 0;
            cache[i][j].dirty = false;
            memset(cache[i][j].data, 0, sizeof(cache[i][j].data));
        }
    }
}

TranslatedAddress translateAddress(Address addr){
    TranslatedAddress ta;
    ta.tag = (addr >> 7) & 0x1FFF;
    ta.setIndex = (addr >> 5) & 0b11;
    ta.offset = addr & 0b11111;
    return ta;
}

Address buildDirtyAddress(unsigned short tag, int setIndex, unsigned char offset) {
    return (tag << 7) | (setIndex << 5) | offset;
}

int checkForCacheHit(short tag, Line line){
    return line.valid && (line.tag == tag);
}

Address getCacheLineAddress(Address addr){
    return (addr&~0x1F);
}


int readWithCache(Address addr){
    TranslatedAddress ta = translateAddress(addr);
    for (int i = 0; i < 2; i++) { //Check for Cache Hit
        if (checkForCacheHit(ta.tag, cache[ta.setIndex][i])) { //If there is a cache hit
            perfCacheHit(addr, ta.setIndex, i); //report cache hit
            cache[ta.setIndex][i].timestamp = ++time; //update the timestamp
            int result;
            memcpy(&result, &cache[ta.setIndex][i].data[ta.offset],4); //copy 4 bytes from cacheline to read result
            return result;
        }
    }
    //If there is no cache hit
    for (int i = 0; i < 2; i++) { //Check for an empty entry
        if (!cache[ta.setIndex][i].valid) { //Found an empty cacheline 
            //Read the cacheline into entry
            readDramCacheLine(getCacheLineAddress(addr),cache[ta.setIndex][i].data); 

            //Fill in management data
            cache[ta.setIndex][i].valid=true;  
            cache[ta.setIndex][i].tag=ta.tag;            

            //Report Cache Miss (Empty)
            perfCacheMiss(addr, ta.setIndex, i, 1); 

            //update timestamp
            cache[ta.setIndex][i].timestamp = ++time;
            
            //copy 4 bytes from cacheline to read result
            int result;
            memcpy(&result,&cache[ta.setIndex][i].data[ta.offset],4);
            return result;
        } 
    }
    //No empty entries, must replace leastRecentlyUsed(LRU)
    int lruIndex;
    if(cache[ta.setIndex][0].timestamp<cache[ta.setIndex][1].timestamp){
        lruIndex=0;
    }
    else{
        lruIndex=1;
    }
    //is that entry dirty? 
    if (cache[ta.setIndex][lruIndex].dirty) {
        Address dirtyAddr = buildDirtyAddress(cache[ta.setIndex][lruIndex].tag, ta.setIndex, 0); //Build dirty address
        writeDramCacheLine(dirtyAddr, cache[ta.setIndex][lruIndex].data); //write dirty cache line back to DRAM
        cache[ta.setIndex][lruIndex].dirty = false;  //Mark entry as no longer dirty
    }
    //Read the cacheline into entry
    readDramCacheLine(getCacheLineAddress(addr),cache[ta.setIndex][lruIndex].data); 

    cache[ta.setIndex][lruIndex].tag=ta.tag; 

    perfCacheMiss(addr, ta.setIndex, lruIndex, 0); //Report Cache Miss 

    cache[ta.setIndex][lruIndex].timestamp = ++time; //Update the timestamp
    //Copy 4 bytes from the cache to read result
    int result;
    memcpy(&result,&cache[ta.setIndex][lruIndex].data[ta.offset],4);
    return result;
}

void writeWithCache(Address addr, int value){
    TranslatedAddress ta = translateAddress(addr);
    for (int i = 0; i < 2; i++) { //check for cache hit
        if (checkForCacheHit(ta.tag, cache[ta.setIndex][i])) { //Have a hit
            perfCacheHit(addr, ta.setIndex, i); //report cache hit
            cache[ta.setIndex][i].timestamp = ++time; //update timestamp
            memcpy(&cache[ta.setIndex][i].data[ta.offset],&value,4); //copy 4 bytes from write parameter to the cache line
            cache[ta.setIndex][i].dirty=true;
            return;
        }
    }//Not a cache hit
    for (int i = 0; i < 2; i++) {//Look for empty entry
        if (!cache[ta.setIndex][i].valid) { //Found an empty entry
            //Read the cacheline into entry
            readDramCacheLine(getCacheLineAddress(addr),cache[ta.setIndex][i].data); 

            //Fill in management data
            cache[ta.setIndex][i].valid = true;
            cache[ta.setIndex][i].tag=ta.tag;            

            //Report Cache Miss (Empty)
            perfCacheMiss(addr, ta.setIndex, i, 1); 
            
            //Update timestamp
            cache[ta.setIndex][i].timestamp = ++time;
            
            //copy 4 bytes from the write parameter to cache line
            memcpy(&cache[ta.setIndex][i].data[ta.offset],&value,4);
            //Mark cache line as dirty
            cache[ta.setIndex][i].dirty = true;
            return;
        } 
    }
    //No empty entries, Find oldest entry
    int lruIndex;
    if(cache[ta.setIndex][0].timestamp<cache[ta.setIndex][1].timestamp){
        lruIndex=0;
    }
    else{
        lruIndex=1;
    }
    //is that entry dirty? 
    if (cache[ta.setIndex][lruIndex].dirty) {
        Address dirtyAddr = buildDirtyAddress(cache[ta.setIndex][lruIndex].tag, ta.setIndex, 0); //Build dirty address
        writeDramCacheLine(dirtyAddr, cache[ta.setIndex][lruIndex].data); //write dirty cache line back to DRAM
        cache[ta.setIndex][lruIndex].dirty = false;  //Mark entry as no longer dirty
    }
    //Read the cacheline into entry. Loads cacheline from DRAM, but we still need to insert the given value
    readDramCacheLine(getCacheLineAddress(addr),cache[ta.setIndex][lruIndex].data); 
    cache[ta.setIndex][lruIndex].tag=ta.tag; 
    //Report Cache Miss 
    perfCacheMiss(addr, ta.setIndex, lruIndex, 0); 

    //Update timeStamp
    cache[ta.setIndex][lruIndex].timestamp = ++time;

    //copy 4 bytes from write parameter to the cacheLine. This is where we insert the given value.
    memcpy(&cache[ta.setIndex][lruIndex].data[ta.offset],&value,4);

    //mark cacheline as dirty
    cache[ta.setIndex][lruIndex].dirty = true;
    return;
}


//Address buildDirtyAddress(unsigned short tag, int setIndex, unsigned char offset) {
void flushCache(){
    perfCacheFlush();
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 2; j++){
            if (cache[i][j].dirty){
                writeDramCacheLine(buildDirtyAddress(cache[i][j].tag,i,0),cache[i][j].data);
            }
            cache[i][j].valid = false;
            cache[i][j].timestamp = 0;
            cache[i][j].dirty = false;
            memset(cache[i][j].data, 0, sizeof(cache[i][j].data));
        }
    }
}
