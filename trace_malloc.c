//
// Created by lexi on 27/07/23.
//

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "trace_malloc.h"
#include "hash_table.h"

bool tracing;
HashTable trace_table;

void start_trace(void) {
    if (tracing) {
        return;
    }

    trace_table = HashTable_new(3);
    tracing = true;
}

void end_trace(void) {
    if (!tracing) {
        return;
    }
    tracing = false;

    HashTableIterator iterator = HashTable_iter(&trace_table);
    while (HashTableIter_hasnext(&iterator)) {
        printf("\033[31;1m");
        AllocationTrace trace = HashTableIter_next(&iterator);
        printf("leaked memory detected! %zu bytes allocated in file %s on line %u\n",
               trace.size, trace.file, trace.line);
        printf("\033[0m");
    }
    HashTable_free(trace_table);
}

void make_trace(const void *ptr, size_t size, const char *file, unsigned int line) {
    AllocationTrace trace = {
        .ptr=ptr,
        .size=size,
        .file=file,
        .line=line,
    };

    HashTable_insert(&trace_table, trace);
}

void *trace_malloc(size_t size, const char *file, unsigned int line) {
    printf("tracing memory allocation; size=%zu, file=%s, line=%d\n", size, file, line);
    if (tracing) {
        void *ptr = malloc(size);
        if (!ptr) return NULL;

        // we suppress warnings here because we are metaprogramming, and
        // sometimes the compiler doesn't know as well as we do
        // (the entire point is that we never dereference this pointer; we only
        // manage it for somebody else)
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
        make_trace(ptr, size, file, line);
        #pragma GCC diagnostic pop

        return ptr;
    } else {
        return malloc(size);
    }
}

void trace_free(void *ptr) {
    if (tracing) {
        HashTable_remove(&trace_table, ptr);
        free(ptr);
    } else {
        free(ptr);
    }
}

void *trace_calloc(size_t nmemb, size_t size, const char *file, unsigned int line) {
    if (tracing) {
        void *ptr = calloc(nmemb, size);
        if (!ptr) return NULL;

        make_trace(ptr, nmemb * size, file, line);

        return ptr;
    } else {
        return calloc(nmemb, size);
    }
}

void *trace_realloc(void *ptr, size_t size, const char *file, unsigned int line) {
    trace_free(ptr);
    return trace_malloc(size, file, line);
}

void *trace_reallocarray(void *ptr, size_t nmemb, size_t size, const char *file, unsigned int line) {
    trace_free(ptr);
    return trace_calloc(nmemb, size, file, line);
}
