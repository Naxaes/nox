#include "allocator.h"


 const allocate_fn allocate_functions[COUNT] = {
        [MALLOC] = malloc_allocate,
        [ARENA]  = arena_allocator_allocate,
        [STACK]  = 0,
        [POOL]   = pool_allocate,
        [BUMP]   = 0,
};

const reallocate_fn reallocate_functions[COUNT] = {
        [MALLOC] = malloc_reallocate,
        [ARENA]  = arena_allocator_reallocate,
        [STACK]  = 0,
        [POOL]   = pool_reallocate,
        [BUMP]   = 0,
};

const deallocate_fn deallocate_functions[COUNT] = {
        [MALLOC] = malloc_deallocate,
        [ARENA]  = arena_allocator_deallocate,
        [STACK]  = 0,
        [POOL]   = pool_deallocate,
        [BUMP]   = 0,
};

const destroy_fn destroy_functions[COUNT] = {
        [MALLOC] = malloc_destroy,
        [ARENA]  = 0,
        [STACK]  = 0,
        [POOL]   = pool_destroy,
        [BUMP]   = 0,
};

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


/****************************************************************************************
 * Malloc Allocator
 ****************************************************************************************/
#undef realloc

#include <stdlib.h>
#include <string.h>

/// Does not support take padding and alignment into account.
size_t mallocated_user_size = 0;

void* malloc_allocate(Allocator allocator, size_t size) {
    (void) allocator;
    void* ptr = malloc(size);

    mallocated_user_size += size;

    return ptr;
}

void* malloc_reallocate(Allocator allocator, size_t size, void* old_ptr, size_t old_size) {
    (void) allocator;
    void* new_ptr = realloc(old_ptr, size);

    mallocated_user_size += size;
    mallocated_user_size -= old_size;

    return new_ptr;
}

void malloc_deallocate(Allocator allocator, void* old_ptr, size_t old_size) {
    (void) allocator;
    mallocated_user_size -= old_size;
    free(old_ptr);
}

#define realloc(allocator, new_size, old_ptr, old_size) realloc_(allocator, new_size, old_ptr, old_size, (Alloc_Location) { __FILE_NAME__, __FUNCTION_NAME__, __LINE__ } )


/****************************************************************************************
 * Arena Allocator
 ****************************************************************************************/
int arena_grow(Arena* arena, size_t size) {
    size_t new_capacity = arena->capacity == 0 ? 8 : 2 * arena->capacity;
    while (arena->size + size > new_capacity) {
        new_capacity *= 2;
    }

    void* data = realloc(arena->parent, new_capacity, arena->data, arena->capacity);
    if (!data) {
        return 0;
    }

    arena->data = data;
    arena->capacity = new_capacity;
    return 1;
}

void* arena_allocate(Arena* arena, size_t size) {
    if (arena->size + size > arena->capacity) {
        if (!arena_grow(arena, size)) {
            return NULL;
        }
    }

    void* ptr = arena->data + arena->size;
    arena->size += size;
    return ptr;
}

void* arena_reallocate(Arena* arena, size_t size, void* old_ptr, size_t old_size) {
    if (size > old_size) {
        if (arena->size + size - old_size > arena->capacity) {
            if (!arena_grow(arena, size - old_size)) {
                return NULL;
            }
        }

        void* ptr = arena->data + arena->size;
        arena->size += size;
        return ptr;
    } else {
        arena->size -= old_size - size;
        return old_ptr;
    }
}

void arena_deallocate(Arena* arena, void* old_ptr, size_t old_size) {
    if (old_ptr == arena->data + arena->size - old_size) {
        arena->size -= old_size;
    }
}

void arena_free_all(Arena* arena) {
    arena->size = 0;
}


/****************************************************************************************
 * Pool Allocator
 ****************************************************************************************/
#define POOL_CHUNK(pool, index) ((uint8_t*)(pool)->chunks + (index) * (pool)->chunk_size)

Pool pool_make(Allocator parent, size_t initial_capacity, size_t chunk_size) {
    Pool pool = { parent, 0, initial_capacity, chunk_size, 0, NULL };
    pool.chunks = alloc(parent, initial_capacity * chunk_size);
    for (size_t i = 0; i < initial_capacity; ++i) {
        *POOL_CHUNK(&pool, i) = i + 1;
    }
    return pool;
}

int pool_grow(Pool* pool, size_t size) {
    if (size > pool->chunk_size) {
        return 0;
    }

    if (pool->size + size > pool->capacity) {
        size_t new_capacity = pool->capacity == 0 ? 8 : 2 * pool->capacity;
        while (pool->size + size > new_capacity) {
            new_capacity *= 2;
        }

        void* data = realloc(pool->parent, new_capacity * pool->chunk_size, pool->chunks, pool->capacity * pool->chunk_size);
        if (!data) {
            return 0;
        }

        pool->chunks = data;
        pool->capacity = new_capacity;
        // NOTE(ted): We could relink all chunks here to create a contiguous free list.
        for (size_t i = pool->size; i < pool->capacity; ++i) {
            *POOL_CHUNK(pool, i) = i + 1;
        }

        pool->first_free = pool->size;
    }
    return 1;
}

void* pool_allocate(void* allocator, size_t size) {
    Pool* pool = (Pool*) allocator;
    if (size > pool->chunk_size) {
        return NULL;
    }

    if (pool->first_free == pool->capacity) {
        if (!pool_grow(pool, size)) {
            return NULL;
        }
    }

    size_t first_free = pool->first_free;
    pool->first_free = *POOL_CHUNK(pool, first_free);
    return POOL_CHUNK(pool, first_free);
}

void* pool_reallocate(void* allocator, size_t size, void* old_ptr, size_t old_size) {
    Pool* pool = (Pool*) allocator;
    (void) pool;
    (void) size;
    (void) old_size;
    return old_ptr;
}

void pool_deallocate(void* allocator, void* old_ptr, size_t old_size) {
    if (old_ptr == NULL)
        return;

    Pool* pool = (Pool*) allocator;
    (void) old_size;
    size_t index = ((uint8_t*)old_ptr - pool->chunks) / pool->chunk_size;
    *POOL_CHUNK(pool, index) = pool->first_free;
    pool->first_free = index;
}

