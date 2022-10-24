#include <stdio.h>
#include <stdlib.h>
#include "linkedlist.h"
#include "hashtable.h"

struct hashtable
{
    // define hashtable struct to use linkedlist as buckets
    linkedlist_t **buckets;
    unsigned int size;
};

/**
 * Hash function to hash a key into the range [0, max_range)
 */
static int hash(int key, int max_range)
{
    // feel free to write your own hash function (NOT REQUIRED)
    key = (key > 0) ? key : -key;
    return key % max_range;
}

hashtable_t *ht_init(int num_buckets)
{
    // create a new hashtable
    hashtable_t *table = malloc(sizeof(hashtable_t));
    table->buckets = malloc(sizeof(linkedlist_t*) * num_buckets);
    for (int i = 0; i < num_buckets; i++){
        table->buckets[i] = ll_init();
    }
    table->size = num_buckets;
    return table;
}

void ht_free(hashtable_t *table)
{
    for (int i = 0; i < table->size; i++){
        ll_free(table->buckets[i]);
    }
    free(table->buckets);
    free(table);
}

void ht_add(hashtable_t *table, int key, int value)
{
    // create a new mapping from key -> value.
    // If the key already exists, replace the value.
    int hashed = hash(key, table->size);
    ll_add(table->buckets[hashed], key, value);
}

int ht_get(hashtable_t *table, int key)
{
    // retrieve the value mapped to the given key.
    // If it does not exist, return 0
    int hashed = hash(key, table->size);
    return ll_get(table->buckets[hashed], key);
}

int ht_size(hashtable_t *table)
{
    // return the number of mappings in this hashtable
    int counter = 0;
    for (int i = 0; i < table->size; i++){
        counter += ll_size(table->buckets[i]);
    }
    return counter;
}
