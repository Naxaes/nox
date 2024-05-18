#include <windows.h>

#include <stdio.h>
#include <string.h>


void* memory_map_executable(void* code, size_t size) {
    void* memory = VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE);
    if (memory == NULL) {
        perror("VirtualAlloc");
        return NULL;
    }

    memcpy(memory, code, size);

    DWORD old_protect_flags;
    if (!VirtualProtect(memory, size, PAGE_EXECUTE_READ, &old_protect_flags)) {
        perror("VirtualProtect");
        VirtualFree(memory, 0, MEM_RELEASE);
        return NULL;
    }

    return memory;
}

void memory_map_free(void* code, size_t size) {
    VirtualFree(code, 0, MEM_RELEASE);
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