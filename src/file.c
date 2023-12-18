#include "os/memory.h"
#include "file.h"

#include <stdio.h>
#include <stdlib.h>


Str read_file(const char* path) {
    FILE* file = fopen(path, "rb");
    if (!file)
        return STR_INVALID;

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = alloc(size + 1);
    if (!buffer)
        return STR_INVALID;

    size_t read = fread(buffer, 1, size, file);
    fclose(file);

    if (read != size) {
        dealloc(buffer);
        return STR_INVALID;
    }

    buffer[size] = '\0';
    return (Str) { size, buffer };
}
