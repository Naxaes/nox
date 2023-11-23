#pragma once
#include <stdlib.h>
#include <stdio.h>

#include <malloc/malloc.h>


void* malloc(size_t s);
void free(void* p);

void* alloc_(const char* file, int line, size_t size);
void dealloc_(const char* file, int line, void* ptr);
size_t memory_in_use(void);
void memory_dump(void);

#define alloc(size) alloc_(__FILE__, __LINE__, size)
#define dealloc(ptr) dealloc_(__FILE__, __LINE__, ptr)
