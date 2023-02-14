#include <mpx/pcb.h>
#include <mpx/io.h>
#include <mpx/serial.h>
#include <string.h>
#include <mpx/memory.h>
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

struct pcb_queue_node
{
    struct pcb* pcb_elem;
    struct pcb_queue_node* p_next;
};

struct pcb_queue {
    struct pcb_queue_node* head;
    struct pcb_queue_node* tail;
    int size;
    const unsigned char type_pri;
};


enum ListStatus {
    DQ_NOELEMENTS   = -1,
    SUCCESS         = 0,
};


#ifndef MPX_PROC_USE_ALT_QUEUES
#define PSTATE_QUEUE_SELECTOR(ps) (PSTATE_EXEC_STATE(ps) + 2 * PSTATE_DPATCH_STATE(ps))
struct pcb_queue pcb_queues[] = {
    { NULL, NULL, 0, 1 }, // ACTIVE READY
    { NULL, NULL, 0, 0 }, // ACTIVE BLOCKED
    { NULL, NULL, 0, 1 }, // SUSPENDED READY
    { NULL, NULL, 0, 0 }, // SUSPENDED BLOCKED
};
#endif


enum ListStatus pcb_queue_enqueue(struct pcb_queue* queue, struct pcb* pcb_in)
{
    struct pcb_queue_node* node = sys_alloc_mem (sizeof (struct pcb_queue_node));
    node->pcb_elem = pcb_in;
    node->p_next = NULL;
    
    if ( queue->size > 1 )
    {
        if ( queue->type_pri )
        {
            struct pcb_queue_node* prevcmpnode = NULL;
            struct pcb_queue_node* cmpnode = queue->head;
            // iterate past greater or equal priorities to insert pcb before other pcbs of the same priority
            while ( (pcb_in->ppri >= cmpnode->pcb_elem->ppri) )
            {
                prevcmpnode = cmpnode;
                cmpnode = cmpnode->p_next;
                if ( cmpnode == NULL )
                {
                    goto pri_iterend;
                }
            }
            // insert between nodes or at head
            if ( cmpnode == queue->head )
            {
                node->p_next = queue->head;
                queue->head = node;
            }
            else    // between nodes
            {
                prevcmpnode->p_next = node;
                node->p_next = cmpnode;
            }
        }
        // insert at tail
        pri_iterend:
        queue->tail->p_next = node;
        queue->tail = node;
    }
    else if ( queue->size == 1 )
    {
        // incoming pcb has greater priority
        if ( (pcb_in->ppri < queue->head->pcb_elem->ppri) && queue->type_pri )
        {
            queue->head->p_next = node;
            queue->tail = queue->head;
            queue->head = node;
        }
        else
        {
            queue->head->p_next = node;
            queue->tail = node;
        }
    }
    else
    {
        queue->head = node;
        queue->tail = node;
    }
    ++queue->size;
    return SUCCESS;
}

enum ListStatus pcb_queue_dequeue(struct pcb_queue* queue, struct pcb** pcb_out)
{
    if (queue->size == 0)
    {
        return DQ_NOELEMENTS;
    }

    struct pcb* pcb_ret = queue->head->pcb_elem;

    struct pcb_queue_node* next = NULL;
    if (queue->size == 1) // queue will be empty after dequeue
    {
        queue->tail = NULL;
    }
    else
    {
        next = queue->head->p_next;
    }
    sys_free_mem (queue->head);
    queue->head = next;
    
    --queue->size;
    *pcb_out = pcb_ret;
    return SUCCESS;
}

void pcb_insert(struct pcb* pcb_in)
{
    enum ProcState pcb_state = pcb_in->pstate;
    
    pcb_queue_enqueue(&pcb_queues[PSTATE_QUEUE_SELECTOR(pcb_state)], pcb_in);
}

/**
@brief
    Allocates a new PCB, initializes it with data provided, and sets state to active-ready.
@param name
    Name for the new process. Must be no larger than the size defined by MPX_PCB_PROCNAME_SZ.
@param cls
    Class of the new process.
@param pri
    Priority of the new process.
@return
    A non-NULL pointer to the created PCB on success, NULL on error during allocation,\
        initialization, or invalid parameters.
*/
struct pcb* pcb_setup(const char* name, enum ProcClass cls, unsigned char pri){

if (sizeof(name) <= MPX_PCB_PROCNAME_SZ){
         struct pcb *PCB = pcb_allocate();
         *PCB -> pname = *name;
          PCB -> pcls = cls;
          PCB -> ppri = pri;
          PCB -> pstate = (ACTIVE & READY);
          return PCB;
} 
    return NULL;

}


/**
@brief
    Frees all memory associated with a given PCB, including its stack.
@param pcb
    A pointer to the pcb to free.
@return
    0 on success or otherwise a negative value upon error.
*/
 int pcb_free(struct pcb* pcb){
      if(sys_free_mem(pcb))
          return 1;

      else return -1 ; 
 }

