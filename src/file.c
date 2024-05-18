#include <stdio.h>
#include <stdlib.h>

#include "allocator.h"
#include "file.h"


Str read_file(const char* path) {
    FILE* file = fopen(path, "rb");
    if (!file)
        return STR_EMPTY;

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = alloc(0, size + 1);
    if (!buffer)
        return STR_EMPTY;

    size_t read = fread(buffer, 1, size, file);
    fclose(file);

    if (read != size) {
        dealloc(0, buffer, size+1);
        return STR_EMPTY;
    }

    buffer[size] = '\0';
    return (Str) { size, buffer };
}
