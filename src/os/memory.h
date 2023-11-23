#pragma once

#include "types.h"


void* memory_map_executable(void* code, size_t size);
void memory_map_free(void* code, size_t size);

