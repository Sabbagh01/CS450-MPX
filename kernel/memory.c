#include <mpx/memory.h>

#include <mpx/vm.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <mpx/syscalls.h>


struct mcb
{
    struct mcb* p_prev;
    struct mcb* p_next;
    size_t blk_size;
};

unsigned char heap_isinit = 0;
struct mcb* free_head = NULL;
struct mcb* alloc_head = NULL;

static void* global_heap = NULL;

void initialize_heap(size_t size)
{
    // there will always be a free block referenced if the heap is initialized
    if (!heap_isinit)
    {
        // kmalloc() only checks for a size less than 0x10000 (65536 bytes) via alloc(), 
        // but this will only called once as the kernel initializes (for the scope of
        // the project) and so wouldn't be dynamically adjusted.
        // Just be sure to not pass anything above this size.
        global_heap = kmalloc(size, 0, NULL);
        // Entire heap will be a free block, so a control block entry will be created at its beginning.
        // The free_list is cyclical so set head block to point to itself for p_prev and p_next.
        free_head = global_heap;
            free_head->p_prev = NULL;
            free_head->p_next = NULL;
            free_head->blk_size = size - sizeof(struct mcb);
        heap_isinit = 1;
    }
    return;
}

void* allocate_memory(size_t size)
{
    // check that the heap was initialized
    if (!heap_isinit)
    {
        return NULL;
    }

    size_t req_full_blk_size = size + sizeof(struct mcb);
    unsigned char blk_replace_free = 0;
    struct mcb* mcb_free_select;
    // check for a free space via first fit strategy
    struct mcb* mcb_free_iter = free_head;
    // iterate through free blocks to find the first from free_next with fit
    while (mcb_free_iter != NULL)
    {
        // check for ability to fit allocation and another mcb to accompany a new block
        // If block size is equal to allocation size, the free mcb can be replaced.
        // Otherwise, if the request would not leave room for an mcb and at least a byte for the next free block
        // then we can also replace the free mcb, as it makes no sense to have useless space.
        //  As this is used later, it is cached for later switching.
        // Note: The first comparison is used to account for unsigned overflow of the second comparison.
        blk_replace_free = (mcb_free_iter->blk_size <= sizeof(struct mcb)) ||
                           (
                                (mcb_free_iter->blk_size - sizeof(struct mcb) <= size) &&
                                (mcb_free_iter->blk_size >= size)
                           );

        if ((mcb_free_iter->blk_size >= req_full_blk_size) || blk_replace_free)
        {
            // alias for OOS
            mcb_free_select = mcb_free_iter;
            goto found_free_block;
        }
        mcb_free_iter = mcb_free_iter->p_next;
    }
    // if, by the end of this loop, no free mcb is selected then no free block can fit request
    return NULL;

    found_free_block: ; // Note to members: this fixes parsing shenanigans 
                        // regarding goto labels and subsequent variable definition.
    // locate an adjacent allocated block, if any
    struct mcb* mcb_alloc_select_next = NULL;
    struct mcb* mcb_alloc_select = alloc_head;
    if (alloc_head == NULL)
    {
        goto skip_alloc_find;
    }
    while (1)
    {
        // the the first allocated block ahead of the selected free block
        if (mcb_alloc_select > mcb_free_select)
        {
            mcb_alloc_select_next = mcb_alloc_select;
            mcb_alloc_select = mcb_alloc_select->p_prev;
            break;
        }
        // found end of list
        if (mcb_alloc_select->p_next == NULL)
        {
            break;
        }
        mcb_alloc_select = mcb_alloc_select->p_next;
    }
    skip_alloc_find: ;
    
    size_t allocnew_blk_size;
    // if we aren't replacing, a new free mcb will be made and placed
    if (!blk_replace_free)
    {
        // create new free block ahead of allocation by just moving the old block
        // offset from selected free mcb, account for mcb space and allocation size
        struct mcb* mcb_free_new = (struct mcb*)(((void*)mcb_free_select) + req_full_blk_size);
        // copy old free mcb to new
        *mcb_free_new = *mcb_free_select;
        // ensure adjacent entries point to the new free mcb
        if (mcb_free_new->p_prev != NULL)
        {
            mcb_free_new->p_prev->p_next = mcb_free_new;
        }
        if (mcb_free_new->p_next != NULL)
        {
            mcb_free_new->p_next->p_prev = mcb_free_new;
        }
        mcb_free_new->blk_size -= req_full_blk_size;

        if (mcb_free_select == free_head)
        {
            free_head = mcb_free_new;
        }
        allocnew_blk_size = size;
    }
    else // replace free mcb
    {
        allocnew_blk_size = mcb_free_select->blk_size;
        if (mcb_free_select == free_head)
        {
            free_head = free_head->p_next;
        }
    }

    // operate on mcb_free_select to replace the old free mcb as the allocated mcb
    struct mcb* mcb_alloc_new = mcb_free_select;
    mcb_alloc_new->blk_size = allocnew_blk_size;
    // check if there are other allocated blocks
    // if mcb_alloc_select is NULL, it can mean all the allocated mcbs are before
    // the original free block or there are no allocated mcbs (alloc_head is NULL)
    
    // check that a valid adjacent mcb was selected
    // non-null alloc_select means the list exists
    if (mcb_alloc_select != NULL)
    {
        // insert allocated block into the list of others
        mcb_alloc_new->p_next = mcb_alloc_select_next;
        mcb_alloc_new->p_prev = mcb_alloc_select;
        mcb_alloc_select->p_next = mcb_alloc_new;
        if (mcb_alloc_select_next != NULL)
        {
            mcb_alloc_select_next->p_prev = mcb_alloc_new;
        }
    }
    // check for no other blocks (only one free block)
    else
    {
        alloc_head = mcb_alloc_new;
        mcb_alloc_new->p_next = NULL;
        mcb_alloc_new->p_prev = NULL;
    }

    return mcb_free_select + sizeof(struct mcb);
}

int free_memory(void* ptr)
{
    // check that the heap was initialized
    if (!heap_isinit)
    {
        return 1;
    }

    // Get an mcb pointer for the allocated block, which may or may not be valid
    struct mcb* mcb_alloc_tofree = (struct mcb*)((void*)ptr - sizeof(struct mcb));

    // Attempt to find the mcb pointer in the allocated list
    struct mcb* mcb_alloc_iter = alloc_head;
    while (mcb_alloc_iter != NULL)
    {
        //Check if desired allocated block has been found
        if (mcb_alloc_iter == mcb_alloc_tofree)
        {
            break;
        }
        //Otherwise continue
        mcb_alloc_iter = mcb_alloc_iter->p_next;
    }
    // If alloc_iter is null, then the block was not found, i.e. invalid pointer to free.
    if (mcb_alloc_iter == NULL)
    {
        return 1;
    }

    // Remove the allocated block mcb from the list
    // Check if selected head of the list
    if (mcb_alloc_tofree == alloc_head)
    {
        // This accounts for head being last allocated block mcb
        alloc_head = mcb_alloc_tofree->p_next;
    }
    else // Not at the head of the list
    {
        mcb_alloc_tofree->p_prev->p_next = mcb_alloc_tofree->p_next;
    }
    // Check if not selected end of the list
    if (mcb_alloc_tofree->p_next != NULL)
    {
        mcb_alloc_tofree->p_next->p_prev = mcb_alloc_tofree->p_prev;
    }
    // Allocated block is now removed from the allocated list
    // Merge selected block into adjacent free spaces
    struct mcb* mcb_free_select_next = NULL;
    struct mcb* mcb_free_select;
    // Find free blocks adjacent to the block to free
    // Check for no free space
    if (free_head == NULL)
    {
        goto skip_free_find;
    }
    struct mcb* mcb_free_iter = free_head;
    while (1)
    {
        if (mcb_alloc_tofree < mcb_free_iter)
        {
            mcb_free_select_next = mcb_free_iter;
            mcb_free_select = mcb_free_iter->p_prev;
            break;
        }
        // found end of list, no free blocks
        if (mcb_free_iter->p_next == NULL)
        {
            mcb_free_select = mcb_free_iter;
            break;
        }
        mcb_free_iter = mcb_free_iter->p_next;
    }
    skip_free_find: ;

    // If no free blocks exist, just convert the mcb to a free block
    if (free_head == NULL)
    {
        free_head = mcb_alloc_tofree;
        mcb_alloc_tofree->p_next = NULL;
        mcb_alloc_tofree->p_prev = NULL;
        return 0;
    }

    struct mcb* mcb_free_merge_select;
    // check adjacency of behind free block (guaranteed to exist at this point)
    if (mcb_free_select != NULL)
    {   if ((void*)mcb_free_select + sizeof(struct mcb) + mcb_free_select->blk_size == mcb_alloc_tofree)
    {
        mcb_free_merge_select = mcb_free_select;
        mcb_free_merge_select->blk_size += mcb_alloc_tofree->blk_size + sizeof(struct mcb);
        if (mcb_free_select_next == NULL)
        {
            // only replace the prior
            mcb_free_merge_select->p_next = NULL;
        }
        // check adjacency of forward free block for forward merge
        if (mcb_free_select_next != NULL)
            if ((void*)mcb_alloc_tofree + sizeof(struct mcb) + mcb_alloc_tofree->blk_size == mcb_free_select_next)
        {
            mcb_free_merge_select->blk_size += mcb_free_select_next->blk_size + sizeof(struct mcb);
            mcb_free_merge_select->p_next = mcb_free_select_next->p_next;
            mcb_free_select_next->p_next->p_prev = mcb_free_merge_select;
        }
    }}
    // check adjacency of forward free block 
    else
    {
        mcb_free_merge_select = mcb_alloc_tofree;
        if (mcb_free_select_next != NULL)
            if ((void*)mcb_alloc_tofree + sizeof(struct mcb) + mcb_alloc_tofree->blk_size == mcb_free_select_next)
        {
            mcb_free_merge_select->blk_size += mcb_free_select_next->blk_size + sizeof(struct mcb);
            mcb_free_merge_select->p_next = mcb_free_select_next->p_next;
            // account for the head and set the head accordingly
            if (mcb_free_select_next == free_head)
            {
                free_head = mcb_free_merge_select;
            }
        }
    }

    return 0;
}

void showAllocatedMemory() {
    struct mcb* currList = alloc_head;
    char allocatedMemoryMsg[] = "\nAllocated Memory: \n";
    write(COM1, STR_BUF(allocatedMemoryMsg));
    
    while(currList != NULL){
        
        //char* printAddress = {0};
       // itoa(printAddress, (int) (currList -> Address));
        char addressMsg[] = "\tAddress: ";
        write(COM1, STR_BUF(addressMsg));
        //char newStr[20];
        //decimalToHexText(newStr, (int) (currList -> Address));
       //char startAddress1[] = newStr;
        //write(COM1, STR_BUF(startAddress1));
        
        char printSize[6];
        itoa(printSize, (int) (currList -> blk_size));
        
        char sizeMsg[] = "\tSize: ";
        char newLine[] = "\n: ";
        write(COM1, STR_BUF(sizeMsg));
        write(COM1, STR_BUF(printSize));
        write(COM1, STR_BUF(newLine));
        
        currList = currList -> p_next;
        
    }
}

void showFreeMemory(){
    struct mcb* currList = free_head;
    char allocatedMemoryMsg[] = "\nAllocated Memory: \n";
    write(COM1, STR_BUF(allocatedMemoryMsg));
    
    while(currList != NULL){
        
        //char* printAddress = {0};
        //itoa(printAddress, (int) (currList -> Address));
        char addressMsg[] = "\tAddress: ";
        write(COM1, STR_BUF(addressMsg));
        //char newStr[20];
       //decimalToHexText(newStr, (int) (currList -> Address));
       //char startAddress1[] = newStr;
       //write(COM1, STR_BUF(startAddress1));
        
        char printSize[6];
        itoa(printSize, (int) (currList -> blk_size));
        
        char sizeMsg[] = "\tSize: ";
        char newLine[] = "\n: ";
        write(COM1, STR_BUF(sizeMsg));
        write(COM1, STR_BUF(printSize));
        write(COM1, STR_BUF(newLine));
        
        currList = currList -> p_next;
        
    }
}
