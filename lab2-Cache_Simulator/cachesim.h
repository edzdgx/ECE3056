#include <stdbool.h>
#ifndef __CACHESIM_H
#define __CACHESIM_H
typedef unsigned long long addr_t;
typedef unsigned long long counter_t;

//defining structure of cache
typedef struct {
    addr_t tag[128];
    bool valid[128];
    bool dirty[128];
    int lru_counter[128];
} cache_set;


void cachesim_init(int block_size, int cache_size, int ways);
void cachesim_access(addr_t physical_add, int write);

void cachesim_print_stats(void);

// helper function
int LRUidx(cache_set cache);
// bool wb_increment();
// addr_t binary_conversion(addr_t num);
#endif
