#include <sys/mman.h>

#include <stdio.h>
#include <string.h>


void* memory_map_executable(void* code, size_t size) {
    void* memory = mmap(NULL, size, PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if (memory == MAP_FAILED) {
        perror("mmap");
        return NULL;
    }

    memcpy(memory, code, size);
    if (mprotect(memory, size, PROT_READ|PROT_EXEC) == -1) {
        munmap(memory, size);
        perror("mprotect");
        return NULL;
    }

    return memory;
}

void memory_map_free(void* code, size_t size) {
    munmap(code, size);
}


void* alloc_(const char* file, int line, size_t size) {
    return malloc(size);
}

void dealloc_(const char* file, int line, void* ptr) {
    free(ptr);
}

size_t memory_in_use(void) {
    return 0;
}

void memory_dump(void) {
}