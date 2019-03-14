//  ./cachesim $tracefile $blocksize $cachesize $associativity
//  ./cachesim trace.random64k 64 8192 4
//  ./cachesim trace.random64k 64 8192 16
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "cachesim.h"
#include "math.h"
#include <stdbool.h>

/**
  * 1. set dirty = 1 for every write
  * 2. increment wb only when a block is evicted
  */

// counter_lru number

/**
  * 1. decide data structure of cache, for blocks
  * 2. how you implement LRU policy
  */

int offset_length;
int tag_length;
int idx_length;

addr_t offset;
addr_t idx;
addr_t tag;

int num_of_sets; // number of index lines
cache_set *cache;


counter_t accesses = 0, hits = 0, misses = 0, writebacks = 0, write_misses = 0, writes = 0;

unsigned int asso;


void cachesim_init(int blocksize, int cachesize, int ways) {

  // allocating memory for data structure
  // calculate the number of sets
  num_of_sets = cachesize / (blocksize * ways);
  idx_length = log2(num_of_sets);
  offset_length = log2(blocksize);
  tag_length = 64 - idx_length - offset_length;

  asso = ways; // pass ways' value to cachesim_access

  // allocating memory space for cache
  cache = (cache_set*)malloc(sizeof(cache_set) * num_of_sets);

  // initialize values for cache
  for (int i = 0; i < num_of_sets; i++) { // for each index

    // initialize each set to initial states
    for (int j = 0; j < ways; j++) { // for each way
      cache[i].tag[j] = -1;
      cache[i].dirty[j] = 0;
      cache[i].valid[j] = 0;
      cache[i].lru_counter[j] = j;
    }

  }

//     printf("offset=%d\n",offset_length);
//     printf("index=%d\n",idx_length);
//     printf("tag=%d\n",tag_length);
}



void cachesim_access(addr_t physical_addr, int write) {
  if (write==1)
    writes++;

  accesses++; // increment accesses every time in this function


  //// calculate actual values of the three parameters
  offset = physical_addr << (idx_length + tag_length) >> (idx_length + tag_length);
  idx = physical_addr << tag_length >> (tag_length + offset_length);
  tag = physical_addr >> (idx_length + offset_length);

  int hitPosition = -1; // initialize as not hit


  // Determine hit position
  for (int i = 0; i < 128; i++) {
    if ((cache[idx].tag[i] != -1) && (cache[idx].valid[i]) && (cache[idx].tag[i] == tag)) {
      hitPosition = i;
    }
  }

  // HIT
  if (hitPosition != -1) {
    hits++;
    if (write) {
      cache[idx].valid[hitPosition] = 1;
      cache[idx].dirty[hitPosition] = 1;
    }
  }

  // MISS
  else {

    misses++;
    if (write) {
      write_misses++;
    }
    hitPosition = LRUidx(cache[idx]); // get the position and update hitPosition

    // if dirty and 1, writeback++
    // if (wb_increment() && write) {
    //   writebacks++;
    //   cache[idx].dirty[hitPosition] = 0;
    // }

    if (cache[idx].dirty[hitPosition]) {
      writebacks++;
      cache[idx].dirty[hitPosition] = 0;
    }

    // update parameters accordingly
    cache[idx].tag[hitPosition] = tag;
    cache[idx].valid[hitPosition] = 1;
    if (write)
      cache[idx].dirty[hitPosition] = 1;
  }

  // UPDATE LRU COUNTER
  for (int i = 0; i < asso; i++) {
    if (cache[idx].lru_counter[i] != -1) {
      if (cache[idx].lru_counter[i] > cache[idx].lru_counter[hitPosition]) {
        cache[idx].lru_counter[i]--;
      }
    }
  }

  cache[idx].lru_counter[hitPosition] = asso - 1;
}




void cachesim_print_stats() {
//   printf("%llu, %llu, %llu\n", tag, idx, offset);
//   printf("%i, %i, %i\n", tag_length, idx_length, offset_length);
  // FILE *fp;
  // fp = fopen("output1.csv", "a");
  // //fprintf(fp, "%llu, %llu, %llu, %llu, %llu, %llu\n", accesses, hits, misses, writebacks, write_misses, writes);
  printf("%llu, %llu, %llu, %llu, %llu, %llu\n", accesses, hits, misses, writebacks, write_misses, writes);
  // fclose(fp);
}

// helper function
int LRUidx(cache_set cache) {
    for (int i = 0; i < asso; i++) {
        if (cache.lru_counter[i] == 0)
        return i;
    }
    return -1;
}

// bool wb_increment() {
//   for (int i = 0; i < asso; i++) {
//     if (cache[idx].dirty[i] && (cache[idx].tag[i] != -1)) {
//       return true;
//     }
//   }
//   return false;
// }
// addr_t binary_conversion(addr_t num) {
//   if (num == 0) { return 0; }

//   return (addr_t)((num%2) + 10 * binary_conversion(num/2));
// }
