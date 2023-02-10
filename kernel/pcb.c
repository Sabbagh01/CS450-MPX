#include <memory.h>
#include <mpx/pcb.h>
#include <comhand.h>
#include <mpx/io.h>
#include <mpx/serial.h>
#include <sys_req.h>
#include <string.h>
#include <stdlib.h>


/**
@brief
    Allocate memory for a new PCB.
@return
    A non-NULL pointer to a newly allocated PCB on success. NULL on error during allocation\
        or initialization.
*/
struct pcb* pcb_allocate(void){
     struct pcb* allocate =  sys_alloc_mem(sizeof(struct pcb));
    return allocate;
}
