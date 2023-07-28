#include <stdio.h>
#define TRACE_MALLOC
#include "trace_malloc.h"


int main(void) {
    start_trace();

    // leak some memory
    printf("\nleaking...\n");
    int *memory_leak = (int *) malloc(64);
    printf("dereferencing... \n");
    *memory_leak = 1;

    // don't leak some memory
    printf("\nnot leaking...\n");
    int *memory_not_leak = (int *) calloc(4, sizeof(int));
    *memory_not_leak = 2;
    free(memory_not_leak);

    end_trace();
}
