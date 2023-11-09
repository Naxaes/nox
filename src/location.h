#pragma once
#include "types.h"

typedef struct {
    i32 row;
    i32 column;
} Location;

static inline Location location_of(const char* source, size_t index) {
    Location location = { 1, 1 };
    for (size_t i = 0; i < index; ++i) {
        if (source[i] == '\n') {
            location.row += 1;
            location.column = 1;
        } else if (source[i] == '\t') {
            location.column += 4;
        } else {
            location.column += 1;
        }
    }
    return location;
}
