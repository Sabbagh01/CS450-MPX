#include <mpx/memory.h>

#include <mpx/vm.h>
#include <stddef.h>
#include <stdint.h>
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
            free_head->p_prev = global_heap;
            free_head->p_next = global_heap;
            free_head->blk_size = size;
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

    size_t full_blk_size = size + sizeof(struct mcb);
    unsigned char blk_replace_free = 0;
    struct mcb* mcb_free_select;
    // check for a free space via first fit strategy
    struct mcb* mcb_free_iter = free_head;
    // iterate through free blocks to find the first from free_next with fit
    do
    {
        // check for ability to fit allocation and another mcb to accompany a new block
        // If block size is equal to allocation size, the free mcb can be replaced.
        //  As this is exclusive to the other comparison, it is cached for later switching.
        blk_replace_free = (mcb_free_iter->blk_size == size);
        if ((mcb_free_iter->blk_size >= full_blk_size)
            || blk_replace_free)
        {
            // alias for OOS
            mcb_free_select = mcb_free_iter;
            goto found_free_block;
        }
        mcb_free_iter = mcb_free_iter->p_next;
    }
    while (mcb_free_iter != NULL);
    // if, by the end of this loop, no free mcb is selected then no free block can fit request
    return NULL;

    found_free_block: ; // Note to members: this fixes parsing shenanigans 
                        // regarding goto labels and subsequent variable definition.
    // locate an adjacent allocated block
    struct mcb* mcb_alloc_select = alloc_head;
    while (mcb_alloc_select != NULL)
    {
        // the the first allocated block ahead of the selected free block
        if (mcb_alloc_select > mcb_free_select)
        {
            break;
        }
        mcb_alloc_select = mcb_alloc_select->p_next;
    }

    // if we aren't replacing, a new free mcb will be made and placed
    if (!blk_replace_free)
    {
        // create new free block ahead of allocation by just moving the old block
        // offset from selected free mcb, account for mcb space and allocation size
        struct mcb* mcb_free_new = (struct mcb*)(((void*)mcb_free_select) + full_blk_size);
        // copy old free mcb to new
        *mcb_free_new = *mcb_free_select;
        // ensure adjacent entries point to the new free mcb
        mcb_free_new->p_prev->p_next = mcb_free_new;
        mcb_free_new->p_next->p_prev = mcb_free_new;
        mcb_free_new->blk_size = mcb_free_select->blk_size - full_blk_size;   
    }
    else
    {
        full_blk_size = size;
    }

    // operate on mcb_free_select to replace the old free mcb as the allocated mcb
    mcb_free_select->blk_size = full_blk_size;
    // check if there are other allocated blocks
    // if mcb_alloc_select is NULL, it can mean all the allocated mcbs are before
    // the original free block or there are no allocated mcbs (alloc_head is NULL)
    if ((mcb_alloc_select != NULL) || ((alloc_head != NULL) && (mcb_alloc_select == NULL)))
    {
        // insert allocated block into the list of others
        mcb_free_select->p_next = mcb_alloc_select;
        mcb_free_select->p_prev = mcb_alloc_select->p_prev;
        mcb_alloc_select->p_prev->p_next = mcb_free_select;
        mcb_alloc_select->p_prev = mcb_free_select;
    }
    // check for no other blocks (only one free block)
    else
    {
        alloc_head = mcb_free_select;
        mcb_free_select->p_next = NULL;
        mcb_free_select->p_prev = NULL;
        mcb_free_select->blk_size = full_blk_size;
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

    //Find the allocated block
    struct mcb* mcb_allocated_blk = ptr - sizeof(struct mcb);

    //Remove from allocated list
    struct mcb* alloc_iter = alloc_head;
    while(alloc_iter != NULL){
        //Check if desired allocated block has been found
        if(alloc_iter == mcb_allocated_blk){
            break;
        }
        //Otherwise continue
        alloc_iter = alloc_iter->p_next;
    }

    //If alloc_iter is null, then the block was not found
    if(alloc_iter == NULL){
        static const char blockDNE[] = "The given allocated block does not exist\r\n";
        write(COM1, STR_BUF(blockDNE));
        return 1;
    } if (alloc_iter == alloc_head){
        //If alloc_iter is the head, then the alloc head will become null, add to free list
        alloc_head = NULL;
    } else if (alloc_iter->p_next == NULL){
        //if alloc_iter is tail, remove tail, make new tail
        alloc_iter->p_prev->p_next = NULL;
    } else {
        //Must be in middle, remove from list
        alloc_iter->p_prev->p_next = alloc_iter->p_next;
        alloc_iter->p_next->p_prev = alloc_iter->p_prev;
    }  

    //Now the block is removed from the allocated list
    //Add the block to the free list

    struct mcb* free_iter = free_head;
    int back_at_head = 0;
    //Find the block that is right before the block to free
    while((free_iter != NULL) | (back_at_head == 1)){
        if(free_iter > alloc_iter){
            free_iter = free_iter->p_prev;
            break;
        }
        free_iter = free_iter->p_next;
        if(free_iter->p_next == free_head){
            back_at_head++;
        }
    }

    //Now add the block to free list
    //check if head
    if (free_iter == free_head){
        //If only head is in list
        if(free_head->p_next == NULL){
            alloc_iter->p_prev = free_head;
            alloc_iter->p_next = free_head;
            free_head->p_next = alloc_iter; 
        } else{
        //Not just head in list
            alloc_iter->p_prev = free_head;
            free_head->p_next->p_prev = alloc_iter;
            alloc_iter->p_next = free_head->p_next; 
            free_head->p_next = alloc_iter;
        }
    } else if(back_at_head == 1){
        //means the needs to be freed block is after all allocated blocks
        alloc_iter->p_prev = free_iter;
        alloc_iter->p_next = free_head;
        free_iter->p_next = alloc_iter;
    } else {
        //In between two blocks
        alloc_iter->p_prev = free_iter;
        free_iter->p_next->p_prev = alloc_iter;
        alloc_iter->p_next = free_iter->p_next;
        free_iter->p_next = alloc_iter;
    }

    //Check for adjacent free memory and merge
    //Check in front
    size_t newBlkSize;
    if((alloc_iter + alloc_iter->blk_size + sizeof(struct mcb)) == alloc_iter->p_next){
        //Remove block
        newBlkSize = alloc_iter->blk_size + alloc_iter->p_next->blk_size;
        alloc_iter->p_next->p_next->p_prev = alloc_iter;
        alloc_iter->p_next = alloc_iter->p_next->p_next;
        //Merge size
        alloc_iter->blk_size = newBlkSize;    
    }
    //Check in back
    if((alloc_iter - alloc_iter->p_prev->blk_size - sizeof(struct mcb)) == alloc_iter->p_prev){
        //Remove block
        newBlkSize = alloc_iter->blk_size + alloc_iter->p_prev->blk_size;
        alloc_iter->p_prev->p_next = alloc_iter->p_next;
        alloc_iter->p_next->p_prev = alloc_iter->p_prev;
        //Merge
        alloc_iter->p_prev->blk_size = newBlkSize;
    } 

    ptr = ptr + 1; // jeez, shut up mr. compiler
    return 0;
}
