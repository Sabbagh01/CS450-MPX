#ifndef MPX_MEMORY_H
#define MPX_MEMORY_H

#include <stddef.h>

/**
 @file mpx/memory.h
 @brief MPX-specific dynamic memory functions.
*/

struct mcb
{
    struct mcb* p_prev;
    struct mcb* p_next;
    size_t blk_size;
};

extern struct mcb* free_head;
extern struct mcb* alloc_head;

/**
 @brief
    Initializes the heap space available to MPX and processes.
 @param size
    Size of the global heap to allocate.
*/
void initialize_heap(size_t size);

/**
 @brief
    Requests a memory allocation from the global heap.
 @param size
    The size, in bytes, of the requested allocation.
 @return
    If successful, a pointer to the start address of allocated memory is returned.
    If memory cannot be allocated, or there is another error, the function returns NULL.
*/
void* allocate_memory(size_t size);

/**
 @brief
    Requests a memory allocation from the global heap.
 @param ptr
    A valid pointer to the start address of an allocated memory block.
 @return
    If successful, 0 is returned. If memory cannot be freed, or memory to free
    cannot be found via provided pointer, then -1 is returned.
*/
int free_memory(void* ptr);

#endif // MPX_MEMORY_H
