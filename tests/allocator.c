#include "allocator.h"



/****************************************************************************************
 * Tests
 ****************************************************************************************/
#include <assert.h>


void test_allocation(Allocator allocator) {
    size_t  sizes[] = { 0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024 };
    size_t* end = sizes + sizeof(sizes) / sizeof(*sizes);

    for (size_t* size = sizes; size != end; ++size) {
        void* ptr = alloc(allocator, *size);
        assert(ptr != NULL);
        assert(mallocated_user_size == *size);
        dealloc(allocator, ptr, *size);
    }
}


void test_reallocation(Allocator allocator) {
    size_t  sizes[] = { 0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024 };
    size_t* end = sizes + sizeof(sizes) / sizeof(*sizes);

    void* ptr = NULL;
    size_t old_size = 0;
    for (size_t* size = sizes; size != end; ++size) {
        ptr = realloc(allocator, *size, ptr, old_size);
        assert(ptr != NULL);
        assert(mallocated_user_size == *size);
        old_size = *size;

        ptr = realloc(allocator, *size + 32, ptr, old_size);
        assert(ptr != NULL);
        assert(mallocated_user_size == *size + 32);
        old_size = *size + 32;

        if (*size > 32) {
            ptr = realloc(allocator, *size - 32, ptr, old_size);
            assert(ptr != NULL);
            assert(mallocated_user_size == *size - 32);
            old_size = *size - 32;
        }

    }
    dealloc(allocator, ptr, old_size);
}


void test_pool_allocation(Allocator allocator) {
    size_t  sizes[] = { 0, 1, 2, 4, 8, 16, 32 };
    size_t* end = sizes + sizeof(sizes) / sizeof(*sizes);

    void* ptrs[100] = { 0 };

    for (size_t* size = sizes; size != end; ++size) {
        for (size_t i = 0; i < 100; ++i) {
            void* ptr = alloc(allocator, *size);
            assert(ptr != NULL);
            ptrs[i] = ptr;
        }
        for (size_t i = 0; i < 100; ++i) {
            dealloc(allocator, ptrs[i], *size);
        }
    }
}


void test_pool_reallocation(Allocator allocator) {
    size_t  sizes[] = { 0, 1, 2, 4, 8, 16, 32 };
    size_t* end = sizes + sizeof(sizes) / sizeof(*sizes);

    void* ptrs[100] = { 0 };

    for (size_t* size = sizes; size != end; ++size) {
        for (size_t i = 0; i < 100; ++i) {
            void* ptr = alloc(allocator, *size);
            ptr = realloc(allocator, 32-*size, ptr, *size);
            assert(ptr != NULL);
            ptrs[i] = ptr;
        }
        for (size_t i = 0; i < 100; ++i) {
            dealloc(allocator, ptrs[i], 32-*size);
        }
    }
}


int main(void) {
    Allocator heap_allocator = { 0 };
    {
        test_allocation(heap_allocator);
        test_reallocation(heap_allocator);

        Pool pool = pool_make(heap_allocator, 1024, 32);
        Allocator pool_allocator = pool_allocator_make(&pool);
        {
            test_pool_allocation(pool_allocator);
            test_pool_reallocation(pool_allocator);
        }
        destroy(pool_allocator);
    }
    destroy(heap_allocator);
    assert(mallocated_user_size == 0);
}













