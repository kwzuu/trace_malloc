#pragma once

#include <stddef.h>

typedef struct {
    const void *ptr;
    size_t size;
    const char *file;
    unsigned int line;
} AllocationTrace;

void start_trace(void);
void end_trace(void);

void *trace_malloc(size_t size, const char *file, unsigned int line);
void *trace_calloc(size_t nmemb, size_t size, const char *file, unsigned int line);
void *trace_realloc(void *ptr, size_t size, const char *file, unsigned int line);
void *trace_reallocarray(void *ptr, size_t nmemb, size_t size, const char *file, unsigned int line);

void trace_free(void *ptr);

#ifdef TRACE_MALLOC
#define malloc(size) trace_malloc((size), __FILE__, __LINE__)
#define calloc(size, nmemb) trace_calloc((size), (nmemb), __FILE__, __LINE__)
#define realloc(ptr, size) trace_realloc((ptr), (size), __FILE__, __LINE__)
#define reallocarray(ptr, nmemb, size) trace_reallocarray((ptr), (nmemb), (size), __FILE__, __LINE__)
#define free(ptr) trace_free((ptr))
#endif

