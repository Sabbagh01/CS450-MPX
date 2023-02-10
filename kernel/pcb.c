#include <memory.h>
#include <pcb.h>

/**
@brief
    Allocate memory for a new PCB.
@return
    A non-NULL pointer to a newly allocated PCB on success. NULL on error during allocation\
        or initialization.
*/
struct pcb* pcb_allocate(void){
     pcb* allocate =  Sys_alloc_mem(sizeof(pcb));
    return allocate;
}
