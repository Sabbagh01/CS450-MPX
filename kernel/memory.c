#include <mpx/memory.h>

#include <mpx/vm.h>
#include <stddef.h>
#include <stdint.h>


struct mcb
{
    struct mcb* p_prev;
    struct mcb* p_next;
    size_t bsize;
};

struct mcb* free_head = NULL;
struct mcb* alloc_head = NULL;

static void* global_heap = NULL;

void initialize_heap(size_t size)
{
    // kmalloc only checks for a size less than 0x10000 (65536 bytes) via alloc(), 
    // but this will only called once as the kernel initializes (for the scope of
    // the project) and so wouldn't be dynamically adjusted.
    // Just be sure to not pass anything above this size.
    global_heap = kmalloc(size, 0, NULL);
    free_head = global_heap;
        free_head->p_prev = NULL;
        free_head->p_next = NULL;
        free_head->bsize = size;
    return;
}
