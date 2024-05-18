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
