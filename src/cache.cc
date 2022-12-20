/*******************************************************
                          cache.cc
********************************************************/

#include <stdlib.h>
#include <assert.h>
#include "cache.h"
using namespace std;

Cache::Cache(int s,int a,int b )
{
   ulong i, j;
   reads = readMisses = writes = 0; 
   writeMisses = writeBacks = currentCycle = 0;

   size       = (ulong)(s);
   lineSize   = (ulong)(b);
   assoc      = (ulong)(a);   
   sets       = (ulong)((s/b)/a);
   numLines   = (ulong)(s/b);
   log2Sets   = (ulong)(log2(sets));   
   log2Blk    = (ulong)(log2(b));   
  
   //*******************//
   //initialize your counters here//
   //*******************//
   cc_trans = mem_trans= interventions= invalidations=flushes=bus_upgr=bus_rdx=0;
   wasted_lookups = useful_lookups= total_lookups =0;
   tagMask =0;
   for(i=0;i<log2Sets;i++)
   {
      tagMask <<= 1;
      tagMask |= 1;
   }
   
   /**create a two dimentional cache, sized as cache[sets][assoc]**/ 
   cache = new cacheLine*[sets];
   for(i=0; i<sets; i++)
   {
      cache[i] = new cacheLine[assoc];
      for(j=0; j<assoc; j++) 
      {
         cache[i][j].invalidate();
      }
   }      
   
}

/**you might add other parameters to Access()
since this function is an entry point 
to the memory hierarchy (i.e. caches)**/
ulong Cache::Access(ulong addr,uchar op)
{
   currentCycle++;/*per cache global counter to maintain LRU order 
                    among cache ways, updated on every cache access*/
   //ulong bus_signal;      

    is_read_miss = 0;
    //flush_flag = 0;
   if(op == 'w') writes++;
   else          reads++;
   
   cacheLine * line = findLine(addr);
   if(line == NULL)/*miss*/
   {	
      cacheLine *newline = fillLine(addr);
      if(op == 'w'){
	
	return write(newline, 'm'); 
      }
      else{
	return read(newline, 'm');
      }	
      //if(op == 'w') newline->setFlags(DIRTY);     
   }
   else
   {
      /**since it's a hit, update LRU and update dirty flag**/
      updateLRU(line);
      if(op == 'w'){
	return write(line, 'h'); 
      }
      else{
	return read(line, 'h');
      }
   }
}

/*look up line*/
cacheLine * Cache::findLine(ulong addr)
{
   ulong i, j, tag, pos;
   
   pos = assoc;
   tag = calcTag(addr);
   i   = calcIndex(addr);
  
   for(j=0; j<assoc; j++)
   if(cache[i][j].isValid()) {
      if(cache[i][j].getTag() == tag)
      {
         pos = j; 
         break; 
      }
   }
   if(pos == assoc) {
      return NULL;
   }
   else {
      return &(cache[i][pos]); 
   }
}

/*upgrade LRU line to be MRU line*/
void Cache::updateLRU(cacheLine *line)
{
   line->setSeq(currentCycle);  
}

/*return an invalid line as LRU, if any, otherwise return LRU line*/
cacheLine * Cache::getLRU(ulong addr)
{
   ulong i, j, victim, min;

   victim = assoc;
   min    = currentCycle;
   i      = calcIndex(addr);
   
   for(j=0;j<assoc;j++)
   {
      if(cache[i][j].isValid() == 0) { 
         return &(cache[i][j]); 
      }   
   }

   for(j=0;j<assoc;j++)
   {
      if(cache[i][j].getSeq() <= min) { 
         victim = j; 
         min = cache[i][j].getSeq();}
   } 

   assert(victim != assoc);
   
   return &(cache[i][victim]);
}

/*find a victim, move it to MRU position*/
cacheLine *Cache::findLineToReplace(ulong addr)
{
   cacheLine * victim = getLRU(addr);
   updateLRU(victim);
  
   return (victim);
}

/*allocate a new line*/
cacheLine *Cache::fillLine(ulong addr)
{ 
   ulong tag;
  
   cacheLine *victim = findLineToReplace(addr);
   assert(victim != 0);
   
   if(victim->getFlags() == MODIFIED) {
      writeBack(addr);
      mem_trans++;
   }
      
   tag = calcTag(addr);   
   victim->setTag(tag);
   victim->setFlags(VALID);    
   /**note that this cache line has been already 
      upgraded to MRU in the previous function (findLineToReplace)**/

   return victim;
}
void Cache::print_state(ulong proc_id){
    cacheLine* line = findLine(proc_id);
    if(line == NULL)
      printf("state = INVALID\n");
     else
      printf(" state = %lu\n",line->getFlags());
}
void Cache::printStats(ulong proc_id, ulong flag)
{ 
    if(flag ==0){
   printf("===== Simulation results (Cache %lu) =====\n",proc_id);
   printf("01. number of reads: %lu\n", reads);
   printf("02. number of read misses: %lu\n", readMisses);
   printf("03. number of writes: %lu\n", writes);
   printf("04. number of write misses: %lu\n", writeMisses);
   float miss_rate = ((float)(readMisses+writeMisses)/(reads+writes))*100;
   printf("05. total miss rate: %.2f%%\n",miss_rate);
   printf("06. number of writebacks: %lu\n", writeBacks);
   printf("07. number of cache-to-cache transfers: %lu\n", cc_trans);
   printf("08. number of memory transactions: %lu\n", mem_trans);
   printf("09. number of interventions: %lu\n", interventions);
   printf("10. number of invalidations: %lu\n", invalidations);
   printf("11. number of flushes: %lu\n", flushes);
   printf("12. number of BusRdX: %lu\n", bus_rdx);
   printf("13. number of BusUpgr: %lu\n", bus_upgr);
   if(global_protocol == 3){
   printf("14. number of useful snoops: %lu\n", useful_lookups);
   printf("15. number of wasted snoops: %lu\n", wasted_lookups);
    }
    }
   if(flag ==1){
    if(global_protocol == 3){
    printf("16. number of filtered snoops: %lu\n", total_lookups);
    }
    }
   /****print out the rest of statistics here.****/
   /****follow the ouput file format**************/
}

ulong Cache::read(cacheLine* line, char res){
    if(global_protocol == 0){  
	if(res == 'h'){
	    return NO_ACTION;    
	}	
        else if(res == 'm'){
        is_read_miss = 1;
            mem_trans++;
	    readMisses++;
	    line->setFlags(SHARED);
	    return BUSRD;
        }
    }
    else if(global_protocol == 1){
	if(res == 'h'){
	    return NO_ACTION;
	}
	else if(res == 'm'){
        is_read_miss = 1;
	    mem_trans++;
	    readMisses++;
	    line->setFlags(SHARED);
	    return BUSRD;
	}	
    }
    else if(global_protocol == 2 || global_protocol == 3){
        if(res == 'h'){
            return NO_ACTION;
        }
        if(res == 'm'){
            is_read_miss = 1;
            readMisses++;
            line->setFlags(EXCLUSIVE);
            return BUSRD;
        } 
    }
    return NO_ACTION;
}
ulong Cache::write(cacheLine* line, char res){
    if(global_protocol == 0){
	if(res == 'h'){
	    if(line->getFlags() == SHARED){
	        line->setFlags(MODIFIED);
         	mem_trans++;
         	bus_rdx++;
		return BUSRDX;
	    }
	    else{
	        return NO_ACTION;
	    }    
	}	
        else if(res == 'm'){
      	    writeMisses++;
      	    mem_trans++;
      	    bus_rdx++;
	    line->setFlags(MODIFIED);
	    return BUSRDX;
        }
    }
    else if(global_protocol == 1){
	if(res == 'h'){
	    if(line->getFlags() == SHARED){
		line->setFlags(MODIFIED);
		bus_upgr++;
		return BUSUPGR;
	    }
	    else{
		return NO_ACTION;
	    }
	}
	else if(res == 'm'){
	    writeMisses++;
	    mem_trans++;
	    bus_rdx++;
	    line->setFlags(MODIFIED);
	    return BUSRDX;
	}
    }
    else if(global_protocol == 2 || global_protocol == 3){
        if(res == 'h'){
            if(line->getFlags() == SHARED){
                line->setFlags(MODIFIED);
                bus_upgr++;
                return BUSUPGR;
            }
            else if(line->getFlags() == EXCLUSIVE){
                line->setFlags(MODIFIED);
                return NO_ACTION;
            }
        }
        if(res == 'm'){
            writeMisses++;
            mem_trans++;
            bus_rdx++;
            line->setFlags(MODIFIED);
            return BUSRDX;
        }        
    }
    return NO_ACTION;
}
ulong Cache::snoop(ulong addr, ulong bus_signal){
    ulong hf_flag =0;
   if(global_protocol == 2)
      total_lookups++;
    cacheLine* line = findLine(addr);
    if(line!=NULL){
        if(global_protocol == 0 || global_protocol == 1){
            if(line->getFlags() == MODIFIED){
                writeBacks++;
                if(bus_signal == BUSRD){
                    interventions++;
                    line->setFlags(SHARED);
                }
                else if(bus_signal == BUSRDX){
                    invalidations++;
                    line->invalidate();
                }
                flushes++;
                mem_trans++;	
            }
            else if(line->getFlags() == SHARED){
                if(bus_signal == BUSRDX){
                    invalidations++;
                    line->invalidate();
                }
                else if(bus_signal == BUSUPGR){
                    invalidations++;
                line->invalidate();
                }
            }
        }
        else if(global_protocol == 2 || global_protocol == 3){
            if(line->getFlags() ==  MODIFIED){
                useful_lookups++;
                if(bus_signal == BUSRD){
                    line->setFlags(SHARED);
                    writeBacks++;
                    interventions++;
                    flushes++;
                    cc_flag = 1;
                    mem_trans++;
                    copy_flag = 1;
                }
                else if(bus_signal == BUSRDX){
                    line->setFlags(INVALID);
                    hf_flag = 1;
                    writeBacks++;
                    flushes++;
                    cc_flag =1;
                    flush_flag = 1;
                    invalidations++;
                    mem_trans++;
                }
            }
            else if(line->getFlags() == EXCLUSIVE){
                useful_lookups++;
                if(bus_signal == BUSRD){
                    line->setFlags(SHARED);
                    interventions++;
                    cc_flag = 1;
                    copy_flag = 1;
                }
                else if(bus_signal == BUSRDX){
                    line->setFlags(INVALID);
                    cc_flag = 1;
                    flush_flag = 1;
                    invalidations++;
                    hf_flag = 1;
                }
            }
            else if(line->getFlags() == SHARED){
                useful_lookups++;
                if(bus_signal == BUSRD){
                    cc_flag = 1;
                    copy_flag = 1;
                }
                else if(bus_signal == BUSRDX){
                    line->setFlags(INVALID);
                    cc_flag = 1;
                    flush_flag = 1;
                    hf_flag = 1;
                    invalidations++;
                }
                else if(bus_signal == BUSUPGR){
                    line->setFlags(INVALID);
                    hf_flag = 1;
                    invalidations++;         
                }
            }
      }
   }
   else{
      if(global_protocol == 2 || global_protocol == 3){
         wasted_lookups++;
         hf_flag =1;
      }
   }
   return hf_flag;
}
void Cache::copy_flag_check(ulong addr){ 
   if(global_protocol == 2 || global_protocol == 3){
        if(is_read_miss == 1){
            cacheLine* line = findLine(addr);
            if(line != NULL){
                if(copy_flag == 1)
                    line->setFlags(SHARED);
                else
                    mem_trans++;
            }
        }
        if(cc_flag == 1)
             cc_trans++;
    }
    if(flush_flag == 1)
        mem_trans--;
}
void Cache::hf_update(ulong addr, ulong flag){
    cacheLine* line = findLine(addr);
    if(flag ==0){
        if(line == NULL){
            cacheLine* newline = fillLine(addr);
            (void)newline;
        }
        else{
            updateLRU(line);
        }
    }
    else if(flag == 1){
        if(line!=NULL){
            line->invalidate();
        }
    }
}
ulong Cache::hf_check(ulong addr){
    cacheLine* line = findLine(addr);
    if(line != NULL){
        total_lookups++;
        updateLRU(line);
        return 1;
    }    
    else{
        return 0;
    }
    
}
