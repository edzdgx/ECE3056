//  ./cachesim $tracefile $blocksize $cachesize $associativity
//  ./cachesim trace.random64k 64 8192 4
//  ./cachesim trace.random64k 64 8192 16
/** Assumptions:
  * global miss = d_miss + i_miss
  * local miss = d_miss + i_miss + L2_miss
  */
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

int i_offset_length;
int i_tag_length;
int i_idx_length;

addr_t offset;
addr_t idx;
addr_t tag;

addr_t i_offset;
addr_t i_idx;
addr_t i_tag;

int num_of_sets; // number of index lines
int id_set_num = 256; // calculated
cache_set *cache;

i_cache *icache;
d_cache *dcache;
cache_set *l2_cache;

counter_t accesses = 0, hits = 0, misses = 0, writebacks = 0;
counter_t i_writes = 0, i_reads = 0, i_fetch = 0;

counter_t i_accesses = 0, i_hits = 0, i_misses = 0, i_writebacks = 0;
counter_t d_accesses = 0, d_hits = 0, d_misses = 0, d_writebacks = 0;
unsigned int asso;


void cachesim_init(int blocksize, int cachesize, int ways) {

  // allocating memory for data structure
  // calculate the number of sets
  num_of_sets = cachesize / (blocksize * ways);
  idx_length = log2(num_of_sets);
  offset_length = log2(blocksize);
  tag_length = 64 - idx_length - offset_length;

  // calculate i-cache / d-cache length
  i_idx_length = log2(id_set_num);
  i_offset_length = log2(64);
  i_tag_length = 64 - i_idx_length - i_offset_length;

  asso = ways; // pass ways' value to cachesim_access

  // allocating memory space for i, d, l2 caches
  icache = (i_cache*)malloc(sizeof(i_cache) * id_set_num);
  dcache = (d_cache*)malloc(sizeof(d_cache) * id_set_num);
  l2_cache = (cache_set*)malloc(sizeof(cache_set) * num_of_sets);

  // initialize values for l2_cache
  for (int i = 0; i < num_of_sets; i++) { // for each index
    // initialize each set to initial states
    for (int j = 0; j < ways; j++) { // for each way
      l2_cache[i].tag[j] = -1;
      l2_cache[i].dirty[j] = 0;
      l2_cache[i].valid[j] = 0;
      l2_cache[i].lru_counter[j] = j;
    }

  }

  for (int i = 0; i < id_set_num; i++) {
    icache[i].tag = -1;
    icache[i].dirty = 0;
    icache[i].valid = 0;
    dcache[i].tag = -1;
    dcache[i].dirty = 0;
    dcache[i].valid = 0;
  }
}



void cachesim_access(addr_t physical_addr, char wri) {

  // calculate actual values of the three parameters
  offset = physical_addr << (idx_length + tag_length) >> (idx_length + tag_length);
  idx = physical_addr << tag_length >> (tag_length + offset_length);
  tag = physical_addr >> (idx_length + offset_length);
  // calculate actual values of the three parameters
  i_offset = physical_addr << (i_idx_length + i_tag_length) >> (i_idx_length + i_tag_length);
  i_idx = physical_addr << i_tag_length >> (i_tag_length + i_offset_length);
  i_tag = physical_addr >> (i_idx_length + i_offset_length);

  // for debugging purposes
  if (wri == 'w') {
    d_accesses++;
    i_writes++;
    if((dcache[i_idx].tag) == i_tag && dcache[i_idx].valid) {
      d_hits++;
      dcache[i_idx].valid = 1;
      dcache[i_idx].dirty = 1;
    }

    else {
      d_misses++;

      dcache[idx].tag = i_tag;
      dcache[i_idx].valid = 1;
      dcache[i_idx].dirty = 1;
    }
  }

  if (wri == 'r') {
    d_accesses++;
    i_reads++;
    if((dcache[i_idx].tag) == i_tag && dcache[i_idx].valid) {
      d_hits++;
      dcache[i_idx].valid = 1;
      dcache[i_idx].dirty = 0;
    }

    else {
      d_misses++;
      dcache[idx].tag = i_tag;

      dcache[i_idx].valid = 1;
      dcache[i_idx].dirty = 0;
    }
  }

  if (wri == 'i'){
    i_accesses++;
    i_fetch++;

    // i_hit!!!!!!!!!!!
    if((icache[i_idx].tag == i_tag) && icache[i_idx].valid) {
      //printf("hit");
      i_hits++;
      icache[i_idx].valid = 1;
      icache[i_idx].dirty = 0;
    }
    // i_miss!!!!!!!!!!!!!!
    else {
      //printf("miss");
      i_misses++;

      icache[i_idx].tag = i_tag;
      icache[i_idx].valid = 1;
      icache[i_idx].dirty = 0;
    }
  }



  int hitPosition = -1; // initialize as not hit

  //printf("%i\n",i_idx);

  // Determine hit position
  for (int i = 0; i < 128; i++) {
    if ((cache[idx].tag[i] != -1) && (cache[idx].valid[i]) && (cache[idx].tag[i] == tag)) {
      hitPosition = i;
    }
  }

  // // HIT
  // if (hitPosition != -1) {
  //   hits++;
  //   if (wri == 'w') {
  //     cache[idx].valid[hitPosition] = 1;
  //     cache[idx].dirty[hitPosition] = 1;
  //   }
  // }

//   // MISS
//   else {

//     misses++;
//     hitPosition = LRUidx(cache[idx]); // get the position and update hitPosition

//     if (cache[idx].dirty[hitPosition]) {
//       writebacks++;
//       cache[idx].dirty[hitPosition] = 0;
//     }

//     // update parameters accordingly
//     cache[idx].tag[hitPosition] = tag;
//     cache[idx].valid[hitPosition] = 1;
//     if (wri == 'w')
//       cache[idx].dirty[hitPosition] = 1;
//   }

//   // UPDATE LRU COUNTER
//   for (int i = 0; i < asso; i++) {
//     if (cache[idx].lru_counter[i] != -1) {
//       if (cache[idx].lru_counter[i] > cache[idx].lru_counter[hitPosition]) {
//         cache[idx].lru_counter[i]--;
//       }
//     }
//   }

//   cache[idx].lru_counter[hitPosition] = asso - 1;
}




void cachesim_print_stats() {
  printf("w: %llu, r: %llu, i: %llu\n", i_writes, i_reads, i_fetch);
  printf("    access, hit, miss, wb\n");
  printf("L2: %llu, %llu, %llu, %llu\n", accesses, hits, misses, writebacks);
  printf("I:  %llu, %llu, %llu, %llu\n", i_accesses, i_hits, i_misses, i_writebacks);
  printf("D:  %llu, %llu, %llu, %llu\n", d_accesses, d_hits, d_misses, d_writebacks);
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
