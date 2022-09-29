#include <stdio.h>
#include <stdlib.h>
#include "linkedlist.h"
#include "hashtable.h"

struct hashtable
{
    // TODO: define hashtable struct to use linkedlist as buckets
};

/**
 * Hash function to hash a key into the range [0, max_range)
 */
static int hash(int key, int max_range)
{
    // TODO: feel free to write your own hash function (NOT REQUIRED)
    key = (key > 0) ? key : -key;
    return key % max_range;
}

hashtable_t *ht_init(int num_buckets)
{
    // TODO: create a new hashtable
}

void ht_free(hashtable_t *table)
{
    //TODO: free a hashtable from memory
}

void ht_add(hashtable_t *table, int key, int value)
{
    // TODO: create a new mapping from key -> value.
    // If the key already exists, replace the value.
}

int ht_get(hashtable_t *table, int key)
{
    // TODO: retrieve the value mapped to the given key.
    // If it does not exist, return 0
}

int ht_size(hashtable_t *table)
{
    // TODO: return the number of mappings in this hashtable
}
