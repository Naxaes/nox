#include "memory.h"

#include <dlfcn.h>
#include <malloc/malloc.h>


static void* (*libc_malloc)(size_t) = NULL;
static void (*libc_free)(void*) = NULL;

static size_t allocated_memory = 0;

typedef struct {
    const char* file;
    int line;
    size_t size;
    void* ptr;
} MemoryUsage;

static MemoryUsage pointers_in_use[32768] = { 0 };
static size_t pointers_in_use_count = 0;

void* malloc(size_t s) {
    if (libc_malloc == NULL)
        libc_malloc = (void* (*)(size_t)) dlsym(RTLD_NEXT, "malloc");
    return libc_malloc(s);
}

void free(void* p) {
    if (libc_free == NULL)
        libc_free = (void (*)(void*))dlsym(RTLD_NEXT, "free");
    libc_free(p);
}

void* alloc_(const char* file, int line, size_t size) {
        void* ptr = malloc(size);
//        printf("alloc(%s:%d): %p = %zu\n", file, line, ptr, size);
        allocated_memory += size;
        pointers_in_use[pointers_in_use_count++] = (MemoryUsage) { file, line, size, ptr };
        return ptr;
}

void dealloc_(const char* file, int line, void* ptr) {
        size_t size = malloc_size(ptr);
//        printf("dealloc(%s:%d): %p = %zu\n", file, line, ptr, size);
        allocated_memory -= size;
        for (size_t i = 0; i < pointers_in_use_count; i++) {
            if (pointers_in_use[i].ptr == ptr) {
                pointers_in_use[i] = pointers_in_use[--pointers_in_use_count];
                break;
            }
        }
        free(ptr);
}

size_t memory_in_use(void) {
    return allocated_memory;
}

void memory_dump(void) {
    printf("Memory in use: %zu\n", allocated_memory);
    for (size_t i = 0; i < pointers_in_use_count; i++) {
        printf("  %s:%d: %p = %zu\n", pointers_in_use[i].file, pointers_in_use[i].line, pointers_in_use[i].ptr, pointers_in_use[i].size);
    }
}