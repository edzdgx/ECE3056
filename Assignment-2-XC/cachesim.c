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

i_cache *icache;
d_cache *dcache;
cache_set *l2cache;

counter_t accesses = 0, hits = 0, misses = 0, writebacks = 0, writes = 0;
// write_misses = 0, writes = 0;
counter_t i_writes = 0, i_reads = 0, i_fetch = 0;

counter_t i_accesses = 0, i_hits = 0, i_misses = 0, i_writebacks = 0;
counter_t d_accesses = 0, d_hits = 0, d_misses = 0, d_writebacks = 0;
unsigned int asso;

// line_size (64, 128, 256), ways = 8, cache_size = line_size * 256 Kbyte
void L1init(int blocksize, int cachesize, int ways) {
  // calculate the number of sets
  num_of_sets = cachesize / (blocksize * ways);
  idx_length = log2(num_of_sets);
  offset_length = log2(blocksize);
  tag_length = 64 - idx_length - offset_length;

  // calculate i-cache / d-cache length
  i_idx_length = log2(id_set_num);
  i_offset_length = log2(64);
  i_tag_length = 64 - i_idx_length - i_offset_length;

  asso = ways; // pass ways' value to L2access

  // allocating memory space for i, d, l2 caches
  icache = (i_cache*)malloc(sizeof(i_cache) * id_set_num);
  dcache = (d_cache*)malloc(sizeof(d_cache) * id_set_num);
  l2cache = (cache_set*)malloc(sizeof(cache_set) * num_of_sets);

  // initialize values for l2cache
  for (int i = 0; i < num_of_sets; i++) { // for each index
    // initialize each set to initial states
    for (int j = 0; j < ways; j++) { // for each way
      l2cache[i].tag[j] = -1;
      l2cache[i].dirty[j] = 0;
      l2cache[i].valid[j] = 0;
      l2cache[i].lru_counter[j] = j;
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

void L1access(addr_t physical_addr, char wri) {
  // for debugging purposes
  if (wri == 'w')
    i_writes++;
  if (wri == 'r')
    i_reads++;
  if (wri == 'i')
    i_fetch++;

  //// calculate actual values of the three parameters
  i_offset = physical_addr << (i_idx_length + i_tag_length) >> (i_idx_length + i_tag_length);
  i_idx = physical_addr << i_tag_length >> (i_tag_length + i_offset_length);
  i_tag = physical_addr >> (i_idx_length + i_offset_length);

  // I ------------------ FETCH
  if (wri == 'i') {

    i_accesses++; // increment accesses every time in this function
    // i_hit!!!!!!
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
      L2access(physical_addr, wri);

      icache[i_idx].tag = i_tag;
      icache[i_idx].valid = 1;
      icache[i_idx].dirty = 0;
    }
  }

  // for dcache
  else if (wri == 'w' || wri == 'r') {

    d_accesses++;

    // hit!!
    if((dcache[i_idx].tag) == i_tag && dcache[i_idx].valid) {
      d_hits++;

      if (wri == 'w') {
        dcache[i_idx].dirty = 1;
        }
      dcache[i_idx].valid = 1;
    }

    else {
      d_misses++;

      // go check L2 for tag
      L2access(physical_addr, wri);

      if (dcache[i_idx].dirty == 1) {
        dcache[i_idx].dirty = 0;
      }

      dcache[i_idx].tag = i_tag;
      dcache[i_idx].valid = 1;

      if (wri == 'w') {
        dcache[idx].dirty = 1;
      }

    }
  }
}


void L2access(addr_t physical_addr, char wri) {

  accesses++; // increment accesses every time in this function


  //// calculate actual values of the three parameters
  offset = physical_addr << (idx_length + tag_length) >> (idx_length + tag_length);
  idx = physical_addr << tag_length >> (tag_length + offset_length);
  tag = physical_addr >> (idx_length + offset_length);

  if (wri == 'w') {
    writes++;
  }
  int hitPosition = -1; // initialize as not hit


  // Determine hit position
  for (int i = 0; i < 128; i++) {
    if ((l2cache[idx].tag[i] != -1) && (l2cache[idx].valid[i]) && (l2cache[idx].tag[i] == tag)) {
      hitPosition = i;
    }
  }

  // HIT
  if (hitPosition != -1) {

    hits++;

    // set dirty and valid
    if (wri == 'w') {
      l2cache[idx].valid[hitPosition] = 1;
      l2cache[idx].dirty[hitPosition] = 1;
    }
  }

  // MISS
  else {

    misses++;
  // if (write) {
  //   write_misses++;
  // }
    hitPosition = LRUidx(l2cache[idx]); // get the position and update hitPosition

    // if dirty and 1, writeback++, set back to 0
    if (l2cache[idx].dirty[hitPosition]) {
      writebacks++;
      l2cache[idx].dirty[hitPosition] = 0;
    }

    // update parameters accordingly
    l2cache[idx].tag[hitPosition] = tag;
    l2cache[idx].valid[hitPosition] = 1;

    if (wri == 'w')
      l2cache[idx].dirty[hitPosition] = 1;
  }

  // UPDATE LRU COUNTER
  for (int i = 0; i < asso; i++) {
    if (l2cache[idx].lru_counter[i] != -1) {
      if (l2cache[idx].lru_counter[i] > l2cache[idx].lru_counter[hitPosition]) {
        l2cache[idx].lru_counter[i]--;
      }
    }
  }

  l2cache[idx].lru_counter[hitPosition] = asso - 1;
}




void cachesim_print_stats() {
//   printf("%llu, %llu, %llu\n", tag, idx, offset);
//   printf("%i, %i, %i\n", tag_length, idx_length, offset_length);
//   FILE *fp;
//   fp = fopen("output1.csv", "a");
//   fprintf(fp, "%llu, %llu, %llu, %llu, %llu, %llu\n", accesses, hits, misses, writebacks, write_misses, writes);
//   printf("%llu, %llu, %llu, %llu, %llu, %llu\n", accesses, hits, misses, writebacks, write_misses, writes);
//   fclose(fp);

  printf("w: %llu, r: %llu, i: %llu\n", i_writes, i_reads, i_fetch);
  printf("    access, hit, miss, wb\n");
  printf("L2: %llu, %llu, %llu, %llu\n", accesses, hits, misses, writebacks);
  printf("I:  %llu, %llu, %llu, %llu\n", i_accesses, i_hits, i_misses, i_writebacks);
  printf("D:  %llu, %llu, %llu, %llu\n\n", d_accesses, d_hits, d_misses, d_writebacks);
  printf("D miss rate = %f\n", (double) d_misses / (double) (d_accesses));
  printf("I miss rate = %f\n", (double) i_misses / (double) (i_accesses));
  printf("L2 miss rate= %f\n", (double) misses / (double) (accesses));
  printf("Glob m rate = %f\n", (double) misses / (double) (accesses + i_accesses + d_accesses));

}


// helper function
int LRUidx(cache_set cache) {
    for (int i = 0; i < asso; i++) {
        if (cache.lru_counter[i] == 0)
        return i;
    }
    return -1;
}













