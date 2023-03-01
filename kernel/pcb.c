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

int pcb_dequeue(struct pcb_queue* queue, struct pcb** pcb_out) {
    if (queue->head == NULL)
    {
        return 1;
    }

    struct pcb* pcb_ret = queue->head->pcb_elem;

    struct pcb_queue_node* next = NULL;
    if (queue->head->p_next == NULL) // queue will be empty after dequeue
    {
        queue->tail = NULL;
    }
    else
    {
        next = queue->head->p_next;
    }
    sys_free_mem (queue->head);
    queue->head = next;
    
    *pcb_out = pcb_ret;
    return 0;
}

void pcb_insert(struct pcb* pcb_in)
{
    struct pcb_queue* queue = &pcb_queues[PSTATE_QUEUE_SELECTOR(pcb_in->pstate)];
    struct pcb_queue_node* node = pcb_in->pnode;
    
    if (queue->head != NULL)
    {
        if (queue->head->p_next == NULL) // size == 1
        {
            // check incoming pcb has greater priority if selected queue is a priority queue
            if (queue->type_pri && (pcb_in->ppri < queue->head->pcb_elem->ppri))
            {
                // place node at head (higher priority)
                // note head is also tail here so no need to assign
                node->p_next = queue->head;
                queue->head = node;
            }
            else
            {
                queue->head->p_next = node;
                queue->tail = node;
            }
        }
        else // size > 1
        {
            if (queue->type_pri)
            {
                struct pcb_queue_node* prevcmpnode = NULL;
                struct pcb_queue_node* cmpnode = queue->head;
                // iterate past greater or equal priorities to insert pcb before other pcbs of the same priority
                while (pcb_in->ppri >= cmpnode->pcb_elem->ppri)
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
                if (cmpnode == queue->head) // at head
                {
                    node->p_next = queue->head;
                    queue->head = node;
                }
                else  // between nodes
                {
                    prevcmpnode->p_next = node;
                    node->p_next = cmpnode;
                }
            }
            else
            {
                // insert at tail
                pri_iterend:
                queue->tail->p_next = node;
                queue->tail = node;
            }
        }
    }
    else // size == 0
    {
        queue->head = node;
        queue->tail = node;
    }
    return;
}

struct pcb* pcb_allocate(void) {
    struct pcb* pcb_new = sys_alloc_mem(sizeof(struct pcb));
    if (pcb_new != NULL)
    {
        pcb_new->pnode = sys_alloc_mem (sizeof (struct pcb_queue_node));
        if (pcb_new->pnode != NULL)
        {
            pcb_new->pnode->pcb_elem = pcb_new;
            pcb_new->pnode->p_next = NULL;
            
            pcb_new->pstackseg = sys_alloc_mem(MPX_PCB_STACK_SZ);
            if (pcb_new->pstackseg != NULL)
            {
                memset(pcb_new->pstackseg, 0, MPX_PCB_STACK_SZ);
                return pcb_new;
            }
            sys_free_mem(pcb_new->pnode);
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
            ((cls == USER) || (cls == KERNEL))
            && ((pri >= 0) && (pri <= 9))
        )
        {
            // initialize a pcb.
            struct pcb* pcb_new = pcb_allocate();
            if (pcb_new != NULL)
            {
                // initialize pcb fields
                    memcpy(pcb_new->pname, name, namelen);
                    pcb_new->ppri = pri;
                    pcb_new->pstate = ACTIVE | READY | cls;
                return pcb_new;
            }
        }
    } 
    return NULL;
}

int pcb_free(struct pcb* pcb) {
    if(sys_free_mem(pcb->pstackseg) == 0)
    {
        if(sys_free_mem(pcb->pnode) == 0)
        {
            memset(pcb, 0, sizeof(struct pcb));
            if(sys_free_mem(pcb) == 0)
            {
                return 0;
            }
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
        if (queue_curr->head != NULL)
        {
            struct pcb_queue_node* node_temp = queue_curr->head;
            while(1)
            {
                // match name
                if (strcmp(name, node_temp->pcb_elem->pname) == 0) {
                    return node_temp->pcb_elem;
                }
                if (node_temp->p_next == NULL)
                {
                    break;
                }
                node_temp = node_temp->p_next;
            }
        }
    }
    return NULL;
}

int pcb_remove(struct pcb* pcb) {
    // only check the queue where the given pcb may be located according to its state
    struct pcb_queue* queue_curr = &pcb_queues[PSTATE_QUEUE_SELECTOR(pcb->pstate)];
    struct pcb_queue_node* node_temp;
    struct pcb_queue_node* node_rmv;
    
    if (queue_curr->head == NULL)
    {
        return -1;
    }
    // check if pcb resides at head of the queue
    if (queue_curr->head->pcb_elem == pcb)
    {
        node_rmv = queue_curr->head;
        // remove head
        queue_curr->head = queue_curr->head->p_next;
        node_rmv->p_next = NULL;
        return 0;
    }
    if (queue_curr->head->p_next == NULL)
    {
        return -1;
    }
    // node_temp is used to check the next node and modify the current node while
    // keeping the ability to stitch up the queue as a singly linked list.
    node_temp = queue_curr->head;
    // check that node_temp is not next to the tail as we iterate
    while (node_temp->p_next != NULL)
    {
        // after match, remove node and stitch up queue
        if (node_temp->p_next->pcb_elem == pcb)
        {
            // reset tail if the removal node is the last in the queue
            if (node_temp->p_next == queue_curr->tail)
            {
                queue_curr->tail = node_temp;
            }
            node_rmv = node_temp->p_next;
            // remove the node
            node_temp->p_next = node_temp->p_next->p_next;
            node_rmv->p_next = NULL;
            return 0;
        }
        // no match so iterate
        node_temp = node_temp->p_next;
    }
    // node_temp is next to the tail at this point, and thus there are no matches
    return -1;
}
