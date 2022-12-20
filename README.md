# Coherence-Protocols
Modelled a 4 processor system with bus-based coherence and compared different coherence protocol optimizations for the four peer caches. Implemented MSI(without BusUpgr), MSI(with BusUpgr), MESI, and MESI with Snoop Filter and evaluated the effects of coherence optimizations on the amount of cache-to-cache transfers. Also compared the number of useful vs wasted lookups for cache with and without a Snoop Filter.

Read-trace:
Each line in the trace file is one memory transaction by one of the processors. Each transaction consists of three elements: processor(0-3) operation(r,w) address(in hex).
For example, if you read the line 5 w 0xabcd from the trace file, that means processor 5 is writing to the address “0xabcd” in its local cache. This means the request will propagate down to cache 5, and cache 5 will take care of that request.

Command line arguments:\
./smp_cache \<cache_size\> \<assoc\> \<block_size\> \<num_processors\> \<protocol\> \<trace_file\>
(protocol = 0:MSI 1:MSI BusUpgr 2:MESI 3:MESI with Snoop Filter)
