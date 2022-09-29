#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "cache.h"
#include "print_helpers.h"

cache_t *make_cache(int capacity, int block_size, int assoc, enum protocol_t protocol, bool lru_on_invalidate_f)
{
  cache_t *cache = malloc(sizeof(cache_t));
  cache->stats = make_cache_stats();

  cache->capacity = capacity;     // in Bytes
  cache->block_size = block_size; // in Bytes
  cache->assoc = assoc;           // 1, 2, 3... etc.

  // first, correctly set these 5 variables.
  // note: you may find math.h's log2 function useful
  cache->n_cache_line = capacity / block_size;
  cache->n_set = capacity / (assoc * block_size);
  cache->n_offset_bit = log2(block_size);
  cache->n_index_bit = log2(capacity) - log2(assoc) - log2(block_size);
  cache->n_tag_bit = ADDRESS_SIZE - log2(capacity) + log2(assoc);

  // next create the cache lines and the array of LRU bits
  // - malloc an array with n_rows
  // - for each element in the array, malloc another array with n_col

  cache->lines = malloc((cache->n_set) * sizeof(cache_line_t *));

  for (int i = 0; i < cache->n_set; i++)
  {
    cache->lines[i] = malloc(sizeof(cache_line_t) * cache->assoc);
  }

  cache->lru_way = malloc((cache->n_set) * sizeof(int));

  // initializes cache tags to 0, dirty bits to false,
  // state to INVALID, and LRU bits to 0
  for (int i = 0; i < cache->n_set; i++)
  {
    cache->lru_way[i] = 0;
    for (int j = 0; j < cache->assoc; j++)
    {
      cache->lines[i][j].tag = 0;
      cache->lines[i][j].dirty_f = false;
      cache->lines[i][j].state = INVALID;
    }
  }

  cache->protocol = protocol;
  cache->lru_on_invalidate_f = lru_on_invalidate_f;

  return cache;
}

/* Given a configured cache, returns the tag portion of the given address.
 *
 * Example: a cache with 4 bits each in tag, index, offset
 * in binary -- get_cache_tag(0b111101010001) returns 0b1111
 * in decimal -- get_cache_tag(3921) returns 15
 */
unsigned long get_cache_tag(cache_t *cache, unsigned long addr)
{
  // Right shift the address by the number of bits in the offset and index
  return addr >> (cache->n_offset_bit + cache->n_index_bit);
}

/* Given a configured cache, returns the index portion of the given address.
 *
 * Example: a cache with 4 bits each in tag, index, offset
 * in binary -- get_cache_index(0b111101010001) returns 0b0101
 * in decimal -- get_cache_index(3921) returns 5
 */
unsigned long get_cache_index(cache_t *cache, unsigned long addr)
{
  // Generate a bitmask for the length of the index
  unsigned long mask = (1 << cache->n_index_bit) - 1;
  // Shift the address right by the number of bits in the offset
  // and then mask off the bits that are not in the index
  return (addr >> cache->n_offset_bit) & mask;
}

/* Given a configured cache, returns the given address with the offset bits zeroed out.
 *
 * Example: a cache with 4 bits each in tag, index, offset
 * in binary -- get_cache_block_addr(0b111101010001) returns 0b111101010000
 * in decimal -- get_cache_block_addr(3921) returns 3920
 */
unsigned long get_cache_block_addr(cache_t *cache, unsigned long addr)
{
  // Generate a bitmask for the length of the offset
  unsigned long mask = (1 << cache->n_offset_bit) - 1;
  // Mask off bits that are in the offset
  return addr & ~mask;
}

/* this method takes a cache, an address, and an action
 * it proceses the cache access. functionality in no particular order:
 *   - look up the address in the cache, determine if hit or miss
 *   - update the LRU_way, cacheTags, state, dirty flags if necessary
 *   - update the cache statistics (call update_stats)
 * return true if there was a hit, false if there was a miss
 * Use the "get" helper functions above. They make your life easier.
 */
bool access_cache(cache_t *cache, unsigned long addr, enum action_t action)
{
  unsigned long index = get_cache_index(cache, addr);
  unsigned long tag = get_cache_tag(cache, addr);
  cache_line_t *set = cache->lines[index];
  unsigned int way_n = cache->lru_way[index];

  bool writeback_f = false;
  bool upgrade_miss_f = false;

  // This variable will be the line that needs to be written over or the correct
  // line, based on whether it is a hit or not.
  cache_line_t line = set[way_n];

  bool isHit = MISS;

  for (int i = 0; i < cache->assoc; i++)
  {
    if (set[i].tag == tag && set[i].state != INVALID)
    {
      isHit = HIT; // if upgrade miss, change this later
      way_n = i;
      line = set[i];
      break;
    }
  }

  if (action == LOAD || action == STORE)
  {
    // Update the most recently used way as new LRU
    cache->lru_way[index] = (way_n + 1) % (cache->assoc);

  }

  if (cache->protocol == MSI)
  {
    if (action == STORE)
    {
      // upgrade miss
      if (isHit && line.state == SHARED)
      {
        upgrade_miss_f = true;
        isHit = false;
      }

      if (!isHit)
      {
        if (line.dirty_f)
        {
          writeback_f = true;
        }
        line.tag = tag;
        line.dirty_f = true;
        line.state = MODIFIED;
      }
    }
    else if (action == LOAD)
    {
      // Update tag for freshly loaded data
      if (!isHit)
      {
        if (line.dirty_f)
        {
          writeback_f = true;
        }
        line.tag = tag;
        line.dirty_f = false;
        line.state = SHARED;
      }
    }
    else if (action == LD_MISS && isHit && line.state == MODIFIED)
    {
      if (line.dirty_f)
      {
        writeback_f = true;
      }
      line.dirty_f = false;
      line.state = SHARED;
    }
    else if (action == ST_MISS && isHit)
    {
      if (line.dirty_f)
      {
        writeback_f = true;
      }
      line.dirty_f = false;
      line.state = INVALID;
    }
  }
  else // VI and NONE
  {
    if (action == LOAD || action == STORE)
    {
      // Update tag for freshly loaded data
      if (!isHit)
      {
        if (line.dirty_f)
        {
          writeback_f = true;
        }
        line.tag = tag;
        line.dirty_f = false;
        line.state = VALID;
      }
    }
    else if ((action == LD_MISS || action == ST_MISS) && isHit && cache->protocol == VI)
    {
      writeback_f = line.dirty_f;
      line.dirty_f = false;
      line.state = INVALID;
    }

    if (action == STORE)
    {
      line.dirty_f = true;
    }
  }

  cache->lines[index][way_n] = line;

  update_stats(cache->stats, isHit, writeback_f, upgrade_miss_f, action);
  log_set(index);
  log_way(way_n);

  return isHit;
}



