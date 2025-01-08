#include "DRAM.h"
#include "DRAM_Cache.h"
#include "Performance.h"
#include "VirtualMemory.h"
#include <stdbool.h>

typedef struct virtualAddressStruct{
    unsigned int vpn:6;
    unsigned int pageOffset:10;
}virtualAddressStruct;


typedef struct tlbEntry{
    unsigned int vpn:6;
    unsigned int ppn:6;
}tlbEntry;

Address pageTableStart;
tlbEntry tlb[2]={0};
bool vmEnabled=false;
bool roundRobin=false;


void vmEnable(Address pageTable){
    //starting address for page table
    pageTableStart=pageTable;
    vmEnabled=true;
}


void vmDisable(){
    vmEnabled=false;
}


Address buildPhysicalAddress(unsigned int ppn,unsigned int pageOffset){
  return ppn<<10|pageOffset;
}


Address readTlb(virtualAddressStruct vas){
    //tlb hit if address already has a translated ppn
    //No need to store the entire PTE
    for(int i=0;i<2;i++){
        if(tlb[i].vpn==vas.vpn){
            perfTlbHit(vas.vpn);
            return buildPhysicalAddress(tlb[i].ppn,vas.pageOffset);
        }
    }
    perfTlbMiss(vas.vpn);
    return -1;
}


void writeTlb(unsigned int vpn,unsigned int ppn){
    tlb[roundRobin].vpn=vpn;
    tlb[roundRobin].ppn=ppn;
    //Round robin with two elements means replace the one that wasn't replaced last 
    roundRobin=!roundRobin;
}


virtualAddressStruct createVaStruct (Address virtualAddress){
    virtualAddressStruct vas;
    vas.vpn=virtualAddress>>10;
    vas.pageOffset=virtualAddress&0x3FF;
    return vas;
}


unsigned int getPpn(unsigned int vpn){
    unsigned int pageTableEntry=readWithCache(pageTableStart+(4*vpn));
    return (pageTableEntry&0x3F); 
}


// reading and writing of memory
int vmRead(Address addr){
    //If disabled, we treat virtual addresses as physical addresses
    if(!vmEnabled){
        return readWithCache(addr);
    }

    virtualAddressStruct vas=createVaStruct(addr);
    Address physicalAddress=readTlb(vas);
    //-1 means we know we did not get a physical address back
    if(physicalAddress!=-1){
        return readWithCache(physicalAddress);
    }
    else{
        //If we do not have a translation in the tlb, we need to do perform the translation
        perfStartAddressTranslation(addr);
        unsigned int ppn=getPpn(vas.vpn);
        physicalAddress=buildPhysicalAddress(ppn,vas.pageOffset);
        perfEndAddressTranslation(physicalAddress);
        writeTlb(vas.vpn,ppn);
        return readWithCache(physicalAddress);
    }
}


void vmWrite(Address addr,int value){
    if(!vmEnabled){
       writeWithCache(addr,value);
       return;
    }

    virtualAddressStruct vas=createVaStruct(addr);
    Address physicalAddress=readTlb(vas);
    if(physicalAddress!=-1){
        writeWithCache(physicalAddress,value);
    }
    else{
        perfStartAddressTranslation(addr);
        unsigned int ppn=getPpn(vas.vpn);
        physicalAddress=buildPhysicalAddress(ppn,vas.pageOffset);
        perfEndAddressTranslation(physicalAddress);
        writeTlb(vas.vpn,ppn);
        writeWithCache(physicalAddress,value);
    }
}
