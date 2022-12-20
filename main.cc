/*******************************************************
                          main.cc
********************************************************/

#include <stdlib.h>
#include <assert.h>
#include <fstream>
using namespace std;

#include "cache.h"

ulong global_protocol;
int copy_flag,flush_flag,cc_flag;
int is_read_miss;
//ulong bus_signal;

int main(int argc, char *argv[])
{
    
    ifstream fin;
    FILE * pFile;

    if(argv[1] == NULL){
         printf("input format: ");
         printf("./smp_cache <cache_size> <assoc> <block_size> <num_processors> <protocol> <trace_file> \n");
         exit(0);
        }

    ulong cache_size     = atoi(argv[1]);
    ulong cache_assoc    = atoi(argv[2]);
    ulong blk_size       = atoi(argv[3]);
    ulong num_processors = atoi(argv[4]);
    ulong protocol       = atoi(argv[5]); /* 0:MSI 1:MSI BusUpgr 2:MESI 3:MESI Snoop FIlter */
    global_protocol = atoi(argv[5]);
    char *fname        = (char *) malloc(20);
    fname              = argv[6];

    printf("===== 506 Coherence Simulator configuration =====\n");
    // print out simulator configuration here
    printf("L1_SIZE: %lu\n",cache_size);
    printf("L1_ASSOC: %lu\n",cache_assoc);
    printf("L1_BLOCKSIZE: %lu\n",blk_size);
    printf("NUMBER OF PROCESSORS: %lu\n",num_processors);
    if(protocol == 0)
        printf("COHERENCE PROTOCOL: MSI\n");
    if(protocol == 1)
        printf("COHERENCE PROTOCOL: MSI BusUpgr\n");
    if(protocol == 2)
        printf("COHERENCE PROTOCOL: MESI\n");
    if(protocol == 3)
        printf("COHERENCE PROTOCOL: MESI Filter\n");
    printf("TRACE FILE: %s\n",fname);
    // Using pointers so that we can use inheritance */
    Cache** cacheArray = (Cache **) malloc(num_processors * sizeof(Cache));
    Cache** historyFilter = (Cache **) malloc(num_processors * sizeof(Cache));   
    for(ulong i = 0; i < num_processors; i++) {
        if(protocol == 0 || protocol == 1 || protocol == 2 || protocol == 3) {
            cacheArray[i] = new Cache(cache_size, cache_assoc, blk_size);
        }
        if(global_protocol == 3){
            historyFilter[i] = new Cache(16*64,1,64);
        }
    }

    pFile = fopen (fname,"r");
    if(pFile == 0)
    {   
        printf("Trace file problem\n");
        exit(0);
    }
    
    ulong proc;
    char op;
    ulong addr;

    ulong bus_signal;

    int line = 1;
    while(fscanf(pFile, "%lu %c %lx", &proc, &op, &addr) != EOF)
    {
#ifdef _DEBUG
        //printf("%d\n", line);
#endif
        // propagate request down through memory hierarchy
        // by calling cachesArray[processor#]->Access(...)
        copy_flag = 0;
        cc_flag = 0;
        flush_flag = 0;
	    bus_signal = cacheArray[proc]->Access(addr, op);
	    for(ulong i=0; i<num_processors; i++){
		if(i != proc){
            if(global_protocol == 3){
                ulong check = historyFilter[i]->hf_check(addr);
                if(check == 0){
                    ulong inv = cacheArray[i]->snoop(addr, bus_signal);
                    if(inv == 1){
                        historyFilter[i]->hf_update(addr,0);
                    }
                }
            }
            else
		        cacheArray[i]->snoop(addr, bus_signal);
		}
	    }
        if(global_protocol == 3){
                historyFilter[proc]->hf_update(addr,1);
            }
        cacheArray[proc]->copy_flag_check(addr);
        line++;
    }
    fclose(pFile);
    //********************************//
    //print out all caches' statistics //
    //********************************//
    for(ulong i=0;i<num_processors;i++){
	    cacheArray[i]->printStats(i,0);
	    historyFilter[i]->printStats(i,1);
    }
}
