#pragma once

#include "logger.h"


void exit(int) __attribute__((noreturn));

/* NOTE: Allocators should be created on the stack and passed down the call stack.
 *       Exceptions are allocators that are global (e.g. malloc) or managed by some sort.
 *       An allocator should only be returned from a function if it is the same allocator that was passed in.
 *       Allocators created on the stack should be destroyed before the function returns.
 */



// TODO: MOVE/REMOVE
#include <stdio.h>
#ifndef __FUNCTION_NAME__
#   ifdef WIN32   //WINDOWS
#       define __FUNCTION_NAME__   __FUNCTION__
#   else          //*NIX
#       define __FUNCTION_NAME__   __func__
#   endif
#endif


#include <stdint.h>
#include <stddef.h>
#include <string.h>


/****************************************************************************************
 * Allocator
 ****************************************************************************************/
#define ALLOCATOR_ALIGNMENT __attribute__((aligned(16)))

typedef struct Alloc_Location {
    const char* file;
    const char* function;
    int         line;
} Alloc_Location;


typedef void* ALLOCATOR_ALIGNMENT Allocator;
typedef void* (*allocate_fn)(Allocator allocator, size_t size);
typedef void* (*reallocate_fn)(Allocator allocator, size_t size, void* old_ptr, size_t old_size);
typedef void  (*deallocate_fn)(Allocator allocator, void* old_ptr, size_t old_size);
typedef void  (*destroy_fn)(Allocator allocator);
typedef enum Allocator_Type {
    MALLOC = 0,
    ARENA  = 1,
    STACK  = 2,
    POOL   = 3,
    BUMP   = 4,
    COUNT  = 16,
} Allocator_Type;


void* malloc_allocate(Allocator allocator, size_t size);
void* malloc_reallocate(Allocator allocator, size_t size, void* old_ptr, size_t old_size);
void  malloc_deallocate(Allocator allocator, void* old_ptr, size_t old_size);
static inline void malloc_destroy(Allocator allocator) {
    (void) allocator; // Assert that all memory has been deallocated.
}

static inline void* arena_allocator_allocate(Allocator allocator, size_t size);
static inline void* arena_allocator_reallocate(Allocator allocator, size_t size, void* old_ptr, size_t old_size);
static inline void  arena_allocator_deallocate(Allocator allocator, void* old_ptr, size_t old_size);

void* pool_allocate(Allocator allocator, size_t size);
void* pool_reallocate(Allocator allocator, size_t size, void* old_ptr, size_t old_size);
void  pool_deallocate(Allocator allocator, void* old_ptr, size_t old_size);
static inline void pool_destroy(Allocator allocator);


extern const allocate_fn allocate_functions[COUNT];
extern const reallocate_fn reallocate_functions[COUNT];
extern const deallocate_fn deallocate_functions[COUNT];
extern const destroy_fn destroy_functions[COUNT];

/// Allocates memory using the given allocator.
/// If the allocation fails, the allocator is left unchanged.
#define alloc(allocator, size) alloc_(allocator, size, (Alloc_Location) { __FILE_NAME__, __FUNCTION_NAME__, __LINE__ } )

/// Reallocates memory using the given allocator.
/// If old_ptr is NULL, it behaves like alloc.
/// If new_size is less than old_size, the memory is truncated, keeping the data up to new_size.
/// If new_size is greater than old_size, the memory is extended or reallocated, and the data up to old_size is preserved.
/// If realloc fails, the allocator and old_ptr are left unchanged.
#define realloc(allocator, new_size, old_ptr, old_size) realloc_(allocator, new_size, old_ptr, old_size, (Alloc_Location) { __FILE_NAME__, __FUNCTION_NAME__, __LINE__ } )

/// Deallocates memory using the given allocator.
#define dealloc(allocator, old_ptr, old_size) dealloc_(allocator, old_ptr, old_size, (Alloc_Location) { __FILE_NAME__, __FUNCTION_NAME__, __LINE__ } )

/// Deallocates all memory used by the allocator.
#define destroy(allocator) destroy_(allocator, (Alloc_Location) { __FILE_NAME__, __FUNCTION_NAME__, __LINE__ } )


// Can be extended with alloc_aligned, realloc_aligned, free_aligned.
static inline void* alloc_(Allocator allocator, size_t size, Alloc_Location location) {
    uint8_t   type = (uintptr_t)allocator & 15;  // 0b1111
    uintptr_t data = (uintptr_t)allocator & 0xFFFFFFFFFFFFFFF0;
    allocate_fn function = allocate_functions[type];
    void* result =  function((Allocator) data, size);

    if (result == NULL) {
        error(0, "[ERROR] allocation failed, bytes=%zu, func=%s, loc=%s:%d\n", size, location.function, location.file, location.line);
    } else {
//        debug(0, "[ALLOC] ptr=%p, size=%zu, func=%s, loc=%s:%d\n", result, size, location.function, location.file, location.line);
    }

    return result;
}

static inline void* realloc_(Allocator allocator, size_t new_size, void* old_ptr, size_t old_size, Alloc_Location location) {
    uint8_t   type = (uintptr_t)allocator & 15;  // 0b1111
    uintptr_t data = (uintptr_t)allocator & 0xFFFFFFFFFFFFFFF0;
    reallocate_fn function = reallocate_functions[type];
    void* result = function((Allocator) data, new_size, old_ptr, old_size);

    if (result != NULL) {
        debug(0, "old_ptr=%p, old_size=%zu, new_ptr=%p, new_size=%zu, func=%s, loc=%s:%d\n", old_ptr, old_size, result, new_size, location.function, location.file, location.line);
    } else {
        error(0, "reallocation failed, old_ptr=%p, old_size=%zu, new_size=%zu, func=%s, loc=%s:%d\n", old_ptr, old_size, new_size, location.function, location.file, location.line);
    }

    return result;
}

static inline void dealloc_(Allocator allocator, void* old_ptr, size_t old_size, Alloc_Location location) {
    uint8_t   type = (uintptr_t)allocator & 15;  // 0b1111
    uintptr_t data = (uintptr_t)allocator & 0xFFFFFFFFFFFFFFF0;
    deallocate_fn function = deallocate_functions[type];
    function((Allocator) data, old_ptr, old_size);

    debug(0, "[DEALLOC] ptr=%p, size=%zu, func=%s, loc=%s:%d\n", old_ptr, old_size, location.function, location.file, location.line);
}

static inline void destroy_(Allocator allocator, Alloc_Location location) {
    uint8_t   type = (uintptr_t)allocator & 15;  // 0b1111
    uintptr_t data = (uintptr_t)allocator & 0xFFFFFFFFFFFFFFF0;
    destroy_fn function = destroy_functions[type];
    function((Allocator) data);

    debug(0, "[DESTROY] allocator=%p, type=%d, loc=%s:%d\n", allocator, type, location.file, location.line);
}



/****************************************************************************************
 * Malloc Allocator
 ****************************************************************************************/
/// Does not support take padding and alignment into account.
extern size_t mallocated_user_size;


/****************************************************************************************
 * Arena Allocator
 ****************************************************************************************/
typedef struct Arena {
    Allocator   parent;
    size_t      size;
    size_t      capacity;
    uint8_t*    data;
} Arena;

int   arena_grow(Arena* arena, size_t size);
void* arena_allocate(Arena* arena, size_t size);
void* arena_reallocate(Arena* arena, size_t size, void* old_ptr, size_t old_size);
void  arena_deallocate(Arena* arena, void* old_ptr, size_t old_size);
void  arena_free_all(Arena* arena);

static inline void* arena_allocator_allocate(Allocator allocator, size_t size) {
    Arena* arena = (Arena*) allocator;
    return arena_allocate(arena, size);
}

static inline void* arena_allocator_reallocate(Allocator allocator, size_t size, void* old_ptr, size_t old_size) {
    Arena* arena = (Arena*) allocator;
    return arena_reallocate(arena, size, old_ptr, old_size);
}

static inline void arena_allocator_deallocate(Allocator allocator, void* old_ptr, size_t old_size) {
    Arena* arena = (Arena*) allocator;
    arena_deallocate(arena, old_ptr, old_size);
}


/****************************************************************************************
 * Pool Allocator
 ****************************************************************************************/
typedef struct ALLOCATOR_ALIGNMENT Pool {
    Allocator   parent;
    /// Number of allocated chunks.
    size_t      size;
    /// Number of chunks that can be allocated.
    size_t      capacity;
    /// Size in bytes of each chunk.
    size_t      chunk_size;
    /// Index of the first free chunk.
    size_t      first_free;
    uint8_t*    chunks;
} Pool;

Pool  pool_make(Allocator parent, size_t initial_capacity, size_t chunk_size);
int   pool_grow(Pool* pool, size_t size);
void* pool_allocate(void* allocator, size_t size);
void* pool_reallocate(void* allocator, size_t size, void* old_ptr, size_t old_size);
void  pool_deallocate(void* allocator, void* old_ptr, size_t old_size);


static inline void pool_destroy(void* allocator) {
    Pool* pool = (Pool*) allocator;
    dealloc(pool->parent, pool->chunks, pool->capacity * pool->chunk_size);
}

static inline Allocator pool_allocator_make(Pool* pool) {
    return (Allocator)((uintptr_t) pool | POOL);
}

