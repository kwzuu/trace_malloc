#pragma once
#include <stdlib.h>
#include "trace_malloc.h"

typedef struct {
    AllocationTrace *traces;
    size_t entry_count;
    size_t capacity;
} Bucket;

Bucket Bucket_new(void);
void Bucket_append(Bucket *bucket, AllocationTrace trace);

typedef struct {
    Bucket *buckets;
    // we store the log base 2 of the bucket count
    // this is because i & j is slow when j is not a power of 2
    size_t log2_bucket_count;
    size_t item_count;
} HashTable;

HashTable HashTable_new(size_t bucket_count);
void HashTable_free(HashTable table);

void HashTable_insert(HashTable *table, AllocationTrace trace);
bool HashTable_remove(HashTable *table, void *ptr);


typedef struct {
    size_t bucket;
    size_t item;
    const HashTable *table;
} HashTableIterator;

HashTableIterator HashTable_iter(const HashTable *table);

AllocationTrace HashTableIter_next(HashTableIterator *iterator);
bool HashTableIter_hasnext(HashTableIterator *iterator);

size_t hash_pointer(void *ptr);
