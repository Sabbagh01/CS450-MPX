#include <mpx/pcb.h>

#include <mpx/io.h>
#include <mpx/serial.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>


#ifndef MPX_PROC_USE_ALT_QUEUES
struct pcb_queue pcb_queues[] = {
    { NULL, NULL, 1 }, // ACTIVE READY
    { NULL, NULL, 0 }, // ACTIVE BLOCKED
    { NULL, NULL, 1 }, // SUSPENDED READY
    { NULL, NULL, 0 }, // SUSPENDED BLOCKED
};
#endif

struct pcb* pcb_running = NULL;

void pcb_insert(struct pcb* pcb_in)
{
    struct pcb_queue* queue = &pcb_queues[PSTATE_QUEUE_SELECTOR(pcb_in->state)];
    
    if (queue->pcb_head != NULL)
    {
        if (queue->pcb_head->p_next == NULL) // size == 1
        {
            // check if selected queue is a priority queue
            if (queue->ispriority)
            {
                // check incoming pcb has greater priority 
                if (pcb_in->state.pri < queue->pcb_head->state.pri)
                {
                    // place node at head (higher priority)
                    // note head is also tail here so no need to assign
                    pcb_in->p_next = queue->pcb_head;
                    queue->pcb_head = pcb_in;
                }
                else
                {
                    goto insert_tail;
                }
            }
            else // not priority queue (or lesser priority) so place at tail
            {
                insert_tail: ;
                queue->pcb_head->p_next = pcb_in;
                queue->pcb_tail = pcb_in;
            }
        }
        else // size > 1
        {
            if (queue->ispriority)
            {
                struct pcb* prevcmpnode = NULL;
                struct pcb* cmpnode = queue->pcb_head;
                // iterate past greater or equal priorities to insert pcb before other pcbs of the same priority
                while (pcb_in->state.pri >= cmpnode->state.pri)
                {
                    prevcmpnode = cmpnode;
                    cmpnode = cmpnode->p_next;
                    // check if forward node is past the tail
                    if (cmpnode == NULL)
                    {
                        // go to the tail insertion case
                        goto pri_iterend;
                    }
                }
                // loop exits normally if the priority of pcb in reaches the
                //   front of pcbs with it's priority queue
                // insert between nodes or at head
                if (cmpnode == queue->pcb_head) // at head
                {
                    pcb_in->p_next = queue->pcb_head;
                    queue->pcb_head = pcb_in;
                }
                else  // between nodes
                {
                    prevcmpnode->p_next = pcb_in;
                    pcb_in->p_next = cmpnode;
                }
            }
            else
            {
                // insert at tail
                pri_iterend: ;
                queue->pcb_tail->p_next = pcb_in;
                queue->pcb_tail = pcb_in;
            }
        }
    }
    else // size == 0
    {
        queue->pcb_head = pcb_in;
        queue->pcb_tail = pcb_in;
    }
    return;
}

struct pcb* pcb_allocate(void) {
    struct pcb* pcb_new = sys_alloc_mem(sizeof(struct pcb));
    if (pcb_new != NULL)
    {
        pcb_new->pstackseg = sys_alloc_mem(MPX_PCB_STACK_SZ);
        if (pcb_new->pstackseg != NULL)
        {
            memset(pcb_new->pstackseg, 0, MPX_PCB_STACK_SZ);
            return pcb_new;
        }
        sys_free_mem(pcb_new);
    }
    return NULL;
}

struct pcb* pcb_setup(const char* name, enum ProcClassState cls, unsigned char pri) {
    // get the name length and check that it can fit in pcb->pname
    size_t namelen = strlen(name) + 1;
    // check that the name given can fit
    if (namelen <= MPX_PCB_PROCNAME_BUFFER_SZ)
    // check name (after size validation) is unique across all queues
    if(pcb_find(name) == NULL)
    {
        if (
            ((cls == PCB_CLASS_USER) || (cls == PCB_CLASS_SYSTEM))
            && ((pri >= 0) && (pri <= 9))
        )
        {
            // initialize a pcb.
            struct pcb* pcb_new = pcb_allocate();
            if (pcb_new != NULL)
            {
                // initialize pcb fields
                    pcb_new->p_next = NULL;
                    memcpy(pcb_new->name, name, namelen);
                    pcb_new->state.pri = pri;
                    pcb_new->state.exec = PCB_EXEC_READY;
                    pcb_new->state.dpatch = PCB_DPATCH_ACTIVE; 
                    pcb_new->state.cls = cls;
                    pcb_new->pctxt = pcb_new->pstackseg + MPX_PCB_STACK_SZ - 1;
                return pcb_new;
            }
        }
    } 
    return NULL;
}

void pcb_context_init(struct pcb* pcb, void* func, void* fargs, size_t fargc) {
	if (fargc > MPX_PCB_MAX_ARG_SZ)
    {
        return;
    }
    if (fargs != NULL)
    {
        pcb->pctxt = (void*)pcb->pctxt - fargc + 1;
        // + 1 as pctxt after pcb_setup will point to last byte in allocated stack
        memcpy (pcb->pctxt, fargs, fargc);
    }
    // account for return address placeholder, i.e, one word on top of params
    // when context popped, note generated callee code will also push ebp over return address
    pcb->pctxt = (void*)pcb->pctxt - 4;
    // other side of context (stack base, context struct bottom at first word)
    void* pctxt_oppo = pcb->pctxt;
    pcb->pctxt = (void*)pcb->pctxt - sizeof (struct context);
    // alias context (top of struct)
    struct context* pctxt = pcb->pctxt;
    // set up context for the pcb, note pcb->pctxt is on the other side of pctxt
	pctxt -> ss = 0x0010;
	pctxt -> ds = 0x0010;
	pctxt -> es = 0x0010;
	pctxt -> fs = 0x0010;
	pctxt -> gs = 0x0010;
	pctxt -> edi = 0;
	pctxt -> esi = 0;
    // note ebp points to what would be the saved ebp of the caller
	pctxt -> ebp = (uint32_t) pctxt_oppo;
	pctxt -> esp = (uint32_t) pctxt_oppo;
	pctxt -> ebx = 0;
	pctxt -> edx = 0;
	pctxt -> ecx = 0;
	pctxt -> eax = 0;
	pctxt -> eip = (uint32_t) func;
	pctxt -> cs = 0x0008;
	pctxt -> eflags = 0x00000202;
    return;
}

int pcb_free(struct pcb* pcb) {
    if(sys_free_mem(pcb->pstackseg) == 0)
    {
        memset(pcb, 0, sizeof(struct pcb));
        if(sys_free_mem(pcb) == 0)
        {
            return 0;
        }
    }
    return -1;
}

struct pcb* pcb_find(const char* name) {
    // iterate through all queues in order
    for (size_t i = 0; i < sizeof(pcb_queues) / sizeof(struct pcb_queue); ++i)
    {
        struct pcb_queue* queue_curr = &pcb_queues[i];
        // check that the queue is not empty (size > 0)
        if (queue_curr->pcb_head != NULL)
        {
            struct pcb* pcb_iter = queue_curr->pcb_head;
            while(1)
            {
                // match name
                if (strcmp(name, pcb_iter->name) == 0) {
                    return pcb_iter;
                }
                if (pcb_iter->p_next == NULL)
                {
                    break;
                }
                pcb_iter = pcb_iter->p_next;
            }
        }
    }
    return NULL;
}

int pcb_remove(struct pcb* pcb) {
    // only check the queue where the given pcb may be located according to its state
    struct pcb_queue* queue_curr = &pcb_queues[PSTATE_QUEUE_SELECTOR(pcb->state)];
    struct pcb* pcb_rmv;
    
    if (queue_curr->pcb_head == NULL)
    {
        return -1;
    }
    // check if pcb resides at head of the queue
    if (queue_curr->pcb_head == pcb)
    {
        // head is last element
        if (queue_curr->pcb_head == queue_curr->pcb_tail)
        {
            queue_curr->pcb_tail = NULL;
        }
        pcb_rmv = queue_curr->pcb_head;
        // remove head
        queue_curr->pcb_head = queue_curr->pcb_head->p_next;
        pcb_rmv->p_next = NULL;
        return 0;
    }
    // if checked head, must check any subsequent pcbs
    if (queue_curr->pcb_head->p_next == NULL)
    {
        return -1;
    }
    // node_temp is used to check the next node and modify the current node while
    // keeping the ability to stitch up the queue as a singly linked list.
    struct pcb* pcb_iter = queue_curr->pcb_head;
    // check that pcb_iter is not next to the tail as we iterate
    while (pcb_iter->p_next != NULL)
    {
        // check for match
        if (pcb_iter->p_next == pcb)
        {
            // remove node and stitch up queue
            // reset tail if the removal node is the last in the queue
            if (pcb_iter->p_next == queue_curr->pcb_tail)
            {
                queue_curr->pcb_tail = pcb_iter;
            }
            pcb_rmv = pcb_iter->p_next;
            // remove the node
            pcb_iter->p_next = pcb_iter->p_next->p_next;
            pcb_rmv->p_next = NULL;
            return 0;
        }
        // no match so iterate
        pcb_iter = pcb_iter->p_next;
    }
    // node_temp is next to the tail at this point, and thus there are no matches
    return -1;
}
