//
// Created by lexi on 27/07/23.
//
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include "hash_table.h"

// we need to use this to trace allocations, so we're gonna
// hope there arent any memory leaks in the hashset code
#undef TRACE_MALLOC

// find out how big a pointer is
#if UINTPTR_MAX == 0xFFFFFFFF
#define PTR32
#elif UINTPTR_MAX == 0xFFFFFFFFFFFFFFFFu
#define PTR64
#else
#error unsupported pointer size for hashsets, only 32 and 64 bits allowed
#endif

#if defined(PTR32)
size_t hash(void *ptr) {
    size_t x = (size_t) ptr;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}
#elif defined(PTR64)

size_t hash_pointer(void *ptr) {
    size_t x = (size_t) ptr;
    x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
    x = x ^ (x >> 31);
    return x;
}
#endif

Bucket Bucket_new(void) {
    return (Bucket) {
            .capacity = 0,
            .traces = NULL,
            .entry_count = 0,
    };
}

void Bucket_append(Bucket *bucket, AllocationTrace trace) {
    if (bucket->capacity == 0) {
        bucket->capacity = 2;
        bucket->traces = calloc(2, sizeof(AllocationTrace));
    }

    if (bucket->entry_count + 1 >= bucket->capacity) {
        bucket->capacity <<= 1;
        bucket->traces = reallocarray(bucket->traces, bucket->capacity, sizeof(AllocationTrace));
    }

    bucket->traces[bucket->entry_count++] = trace;
}

HashTable HashTable_new(size_t log2_bucket_count) {
    size_t bucket_count = 1 << log2_bucket_count;
    HashTable table = {
            .buckets = calloc(bucket_count, sizeof(Bucket)),
            .log2_bucket_count = log2_bucket_count,
            .item_count = 0,
    };

    for (size_t i = 0; i < bucket_count; i++) {
        table.buckets[i] = Bucket_new();
    }

    return table;
}

void HashTable_insert(HashTable *table, AllocationTrace trace) {
    size_t mask = (1 << table->log2_bucket_count) - 1;
    size_t bucket_index = (size_t) trace.ptr & mask;
    Bucket *bucket = table->buckets + bucket_index;
    table->item_count++;
    Bucket_append(bucket, trace);
}

bool HashTable_remove(HashTable *table, void *ptr) {
    size_t mask = (1 << table->log2_bucket_count) - 1;
    size_t bucket_index = (size_t) ptr & mask;
    Bucket *bucket = table->buckets + bucket_index;
    for (size_t i = 0; i < bucket->entry_count; i++) {
        AllocationTrace *trace = bucket->traces + i;
        // we don't reap removed traces here, only setting them to null
        // this is because it's easier to code (and a little more time-efficient) if we just do it at rehash
        if (trace->ptr == ptr) {
            trace->ptr = NULL;
            table->item_count--;
            return true;
        }
    }
    return false;
}

HashTableIterator HashTable_iter(const HashTable *table) {
    HashTableIterator iterator = {
        .table = table,
        .bucket = 0,
        .item = 0,
    };

    return iterator;
}

bool HashTableIter_hasnext(HashTableIterator *iterator) {
    const HashTable *table = iterator->table;
    const size_t bucket_count = 1 << table->log2_bucket_count;
    while (iterator->bucket < bucket_count) {
        Bucket *bucket = table->buckets + iterator->bucket;
        while (iterator->item < bucket->entry_count) {
            AllocationTrace *candidate = bucket->traces + iterator->item;
            if (candidate->ptr) {
                return true;
            }
            iterator->item++;
        }
        iterator->bucket++;
    }
    return false;
}

AllocationTrace HashTableIter_next(HashTableIterator *iterator) {
    if (HashTableIter_hasnext(iterator)) {
        Bucket *bucket = iterator->table->buckets + iterator->bucket;
        AllocationTrace trace = bucket->traces[iterator->item];
        iterator->item++;
        return trace;
    } else {
        printf("WARNING: HashTableIter_next called without checking hasnext first");
        AllocationTrace empty = {
                .ptr=NULL,
        };
        return empty;
    }
}


unsigned int log2i(size_t i) {
    return 31 - __builtin_clz(i);
}


void HashTable_rehash(HashTable *table) {
    HashTable new_table = HashTable_new(log2i(table->item_count));
    HashTableIterator iterator = HashTable_iter(table);
    while (HashTableIter_hasnext(&iterator)) {
        AllocationTrace trace = HashTableIter_next(&iterator);
        if (trace.ptr) {
            HashTable_insert(&new_table, trace);
        }
    }

    HashTable_free(*table);
    *table = new_table;
}


void HashTable_free(HashTable table) {
    size_t bucket_count = (1 << table.log2_bucket_count);
    for (size_t i = 0; i < bucket_count; i++) {
        Bucket bucket = table.buckets[i];
        if (bucket.traces) {
            free(bucket.traces);
        }
    }
}
