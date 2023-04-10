#include <mpx/memory.h>

#include <mpx/vm.h>
#include <stddef.h>
#include <stdint.h>


struct mcb
{
    struct mcb* p_prev;
    struct mcb* p_next;
   // u32int address;

   // 0 is free, 1 is allocated 
   int type ;
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

void* allocate_memory(size_t size)
{
    // stub

    //memory = block
    //allocation block = freeblock
    struct mcb* memory = free_head;
    struct mcb* allocation_block = NULL;
    while(memory != NULL){
        memory = memory->p_next;
    }
   // allocation_block = memory -> address + size;
    allocation_block ->bsize = size;
    allocation_block -> type = 0;
    allocation_block -> p_next = memory ->p_next;
    allocation_block ->p_prev = memory -> p_prev;
    
    memory -> bsize = size;
    memory -> p_next = alloc_head;
    memory -> p_prev = NULL;
    memory -> type = 1;
    alloc_head = memory;
    return 1;
}

int free_memory(void* ptr)
{
    // stub
    return 0;
}
