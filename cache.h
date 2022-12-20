/*******************************************************
                          cache.h
********************************************************/

#ifndef CACHE_H
#define CACHE_H

#include <cmath>
#include <iostream>

typedef unsigned long ulong;
typedef unsigned char uchar;
typedef unsigned int uint;
extern ulong global_protocol;
extern int copy_flag,flush_flag;
extern int is_read_miss;
extern int cc_flag;

/****add new states, based on the protocol****/
enum {
   INVALID = 0,
   VALID,
   DIRTY,
   MODIFIED,
   SHARED,
   BUSRD,
   BUSRDX,
   BUSUPGR,
   EXCLUSIVE,
   NO_ACTION
};

class cacheLine 
{
protected:
   ulong tag;
   ulong Flags;   // 0:invalid, 1:valid, 2:dirty 
   ulong seq; 
 
public:
   cacheLine()                { tag = 0; Flags = 0; }
   ulong getTag()             { return tag; }
   ulong getFlags()           { return Flags;}
   ulong getSeq()             { return seq; }
   void setSeq(ulong Seq)     { seq = Seq;}
   void setFlags(ulong flags) {  Flags = flags;}
   void setTag(ulong a)       { tag = a; }
   void invalidate()          { tag = 0; Flags = INVALID; } //useful function
   bool isValid()             { return ((Flags) != INVALID); }
};

class Cache
{
protected:
   ulong size, lineSize, assoc, sets, log2Sets, log2Blk, tagMask, numLines;
   ulong reads,readMisses,writes,writeMisses,writeBacks;
   ulong cc_trans, mem_trans, interventions, invalidations,flushes,bus_upgr,bus_rdx;
   ulong wasted_lookups, useful_lookups,total_lookups;

   //******///
   //add coherence counters here///
   //******///

   cacheLine **cache;
   ulong calcTag(ulong addr)     { return (addr >> (log2Blk) );}
   ulong calcIndex(ulong addr)   { return ((addr >> log2Blk) & tagMask);}
   ulong calcAddr4Tag(ulong tag) { return (tag << (log2Blk));}
   
public:
    ulong currentCycle;  
     
    Cache(int,int,int);
   ~Cache() { delete cache;}
   
   cacheLine *findLineToReplace(ulong addr);
   cacheLine *fillLine(ulong addr);
   cacheLine * findLine(ulong addr);
   cacheLine * getLRU(ulong);
   
   ulong getRM()     {return readMisses;} 
   ulong getWM()     {return writeMisses;} 
   ulong getReads()  {return reads;}       
   ulong getWrites() {return writes;}
   ulong getWB()     {return writeBacks;}
   
   void writeBack(ulong) {writeBacks++;}
   ulong Access(ulong,uchar);
   void printStats(ulong proc_id, ulong flag);
   void updateLRU(cacheLine *);

   //******///
   //add other functions to handle bus transactions///


   ulong read(cacheLine* line, char res);
   ulong write(cacheLine* line, char res);
   ulong snoop(ulong addr, ulong bus_signal);
   void copy_flag_check(ulong addr);
   void print_state(ulong proc_id);
   void hf_update(ulong addr,ulong flag);
   ulong hf_check(ulong addr);
   //******///

};

#endif
