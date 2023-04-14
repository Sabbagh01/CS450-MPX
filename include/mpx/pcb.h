#ifndef MPX_PCB_H
#define MPX_PCB_H

#include <stddef.h>
#include <mpx/device.h>
#include <mpx/context.h>

/**
 @file mpx/pcb.h
 @brief Facilities for manipulating process control blocks
*/

#define MPX_PCB_PROCNAME_BUFFER_SZ (64)
#define MPX_PCB_PROCNAME_SZ (MPX_PCB_PROCNAME_BUFFER_SZ - 1)

#define MPX_PCB_PROCPRI_MAX (9)

/**
 @brief
    Defines unique process execution state identifiers.
*/
enum ProcExecState {
    BLOCKED     = 0x00,
    READY       = 0x01,
    RUNNING     = 0x02,
};

/**
 @brief
    Defines unique process dispatch state identifiers.
*/
enum ProcDpatchState {
    SUSPENDED   = 0x00,
    ACTIVE      = 0x01,
};

/**
 @brief
    Defines unique process class identifiers.
*/
enum ProcClassState {
    KERNEL      = 0x00,
    USER        = 0x01,
};

#define MPX_PCB_STACK_SZ (4096)

struct pcb_queue_node;

/**
 @struct pcb_state
 @brief
    Defines organization of the state in a PCB using bitfields.
 @var pcb_state::exec
    The current execution state of a process. Should be a value from enum ProcExecState.
 @var pcb_state::dpatch
    The current dispatch state of a process. Should be a value from enum ProcDpatchState.
 @var pcb_state::cls
    The class of a process. Should be a value from ProcClassState.
 @var pcb_state::pri
    The current scheduling priority of a process. The value must be in [0-9]
    where 0 is the highest priority and increasing values indicate decreasing priority.
*/
struct pcb_state {
    unsigned char exec   : 2;
    unsigned char dpatch : 1;
    unsigned char cls    : 1;
    unsigned char pri    : 4;
};

/**
 @struct pcb
 @brief
    Defines a process control block (PCB) structure for maintaining process information
    for a process.
 @var pcb::pname
    The current name of a process
 @var pcb::state
    The state of a PCB. Includes process execution, dispatch, class, and priority.
 @var pcb::pstackseg
    A pointer to the stack section allocated for a process.
 @var pcb::pctxt
    A pointer to a saved context for a process. When the process is not active,
    it will point to a valid context in the allocated stack section,
    which will be situated on top of the stack.
 @var pcb::curr_io
    A pointer to a current I/O operation being done by a process. If not NULL,
    the process is waiting for I/O and will be blocked.
*/
struct pcb {
    char name[MPX_PCB_PROCNAME_BUFFER_SZ];
    struct pcb_queue_node* pnode;
    struct pcb_state state;
    void* pstackseg;
    struct context* pctxt;

    struct iocb* curr_io;
};

/**
 @struct pcb_queue_node
 @brief
    A node to hold queued PCB handles for a `pcb_queue`.
 @var pcb_queue_node::pcb_elem
    A pointer to an existing PCB.
 @var pcb_queue_node::p_next
    A pointer to a next entry in a queue. NULL if there is no next entry.
*/
struct pcb_queue_node
{
    struct pcb* pcb_elem;
    struct pcb_queue_node* p_next;
};

/**
 @struct pcb_queue
 @brief
    A queue to hold queued PCB handles, linked list implementation.
 @var pcb_queue::head
    If the queue is not empty, points to head/front of the queue which can be dequeued. NULL otherwise.
 @var pcb_queue::tail
    If the queue is not empty, points to the tail/back of the queue to help enqueue an element.
    NULL otherwise.
 @var pcb_queue::type_pri
    Identifies whether the queue is a priority queue. If so, inserting via `pcb_insert()`
    might not insert nodes at the tail and will insert based on first priority.
*/
struct pcb_queue {
    struct pcb_queue_node* head;
    struct pcb_queue_node* tail;
    const unsigned char type_pri;
};

#ifndef MPX_PROC_USE_ALT_QUEUES
#define PSTATE_QUEUE_SELECTOR(state) ((state.exec != READY) + 2 * ((state.dpatch != ACTIVE)))
#define QUEUE_SZ (4)

/**
 @var pcb_queues
 @brief
    An array of queues corresponding to process states.
*/
extern struct pcb_queue pcb_queues[QUEUE_SZ];
#endif

/**
 @var pcb_running
 @brief
    Indicates the current running process.
*/
extern struct pcb* pcb_running;

/**
 @brief
    Allocate memory for a new PCB.
 @return
    A non-NULL pointer to a newly allocated PCB on success. NULL on error during allocation
    or initialization.
*/
struct pcb* pcb_allocate(void);

/**
 @brief
    Frees all memory associated with a given PCB, including its stack.
 @param pcb
    A pointer to the pcb to free.
 @return
    If successful, 0 is returned. A negative value will be returned otherwise.
*/
int pcb_free(struct pcb* pcb);

/**
 @brief
    Allocates a new PCB, initializes it with data provided, and sets state to active-ready.
 @param name
    Name string for the new process. Must be a NUL-terminated string and no larger
        than the size defined by MPX_PCB_PROCNAME_SZ.
 @param cls
    Class of the new process.
 @param pri
    Priority of the new process.
 @return
    A non-NULL pointer to the created PCB on success, NULL on error during allocation,
    initialization, or invalid parameters.
*/
struct pcb* pcb_setup(const char* name, enum ProcClassState cls, unsigned char pri);

#define MPX_PCB_MAX_ARG_SZ (32)

/**
 @brief
    Sets up the context for a process.
 @param pcb
    PCB to set up the context for.
 @param func
    Entry point to set for the PCB.
 @param fargs
    A pointer to a buffer of arguments to be copied into the PCB stack to be
        accessed by the entry point. If NULL, fargc is disregarded and the buffer
        is not copied.
 @param fargc
    Size of the buffer pointed to by fargs.
*/
void pcb_context_init(struct pcb* pcb, void* func, void* fargs, size_t fargc);

/**
 @brief
    Searches all process queues for processes with the provided name.
 @param name
    Name of the process to find.
 @return
    A non-NULL pointer to the found PCB on success. NULL if the provided name was\
    not found in any queue.
*/
struct pcb* pcb_find(const char* name);

/**
 @brief
    Inserts a PCB into the appropriate queue based on state and priority.
 @param pcb
    A pointer to the PCB to enqueue.
*/
void pcb_insert(struct pcb* pcb);

/**
 @brief
    Removes a PCB from its current queue without freeing memory or data structures.
 @param pcb
    A pointer to the PCB to dequeue.
 @return
    0 on success, a negative value if there was an error.
*/
int pcb_remove(struct pcb* pcb);

#endif // MPX_PCB_H
