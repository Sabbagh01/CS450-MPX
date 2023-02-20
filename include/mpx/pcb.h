#ifndef MPX_PCB_H
#define MPX_PCB_H

/**
@file mpx/pcb.h
@brief Facilities for manipulating process control blocks
*/

#define MPX_PCB_PROCNAME_SZ (64)
#define MPX_PCB_PROCNAME_MIN (8)

/**
@brief Defines unique process class identifiers
*/
enum ProcClass {
    KERNEL      = 0x00,
    USER        = 0x01,
};

#define DPATCH_SHIFT 7
#define DPATCH_BITS (1 << 7)
#define EXEC_BITS (~DPATCH_BITS)

#define PSTATE_EXEC_STATE(ps)           (ps & EXEC_BITS)
#define PSTATE_DPATCH_STATE(ps)         (ps & DPATCH_BITS)
#define PSTATE_EXEC_STATE_TOVALUE(ps)   (ps & EXEC_BITS)
#define PSTATE_DPATCH_STATE_TOVALUE(ps) ((ps & DPATCH_BITS) >> DPATCH_SHIFT)

/**
@brief Defines unique process state identifiers
*/
enum ProcState {
    // begin execution states
    READY       = 1,
    RUNNING     = 2,
    BLOCKED     = 3,
    // begin dispatch states
    SUSPENDED   = 0 << DPATCH_SHIFT,
    ACTIVE      = 1 << DPATCH_SHIFT,
};

#define MPX_PCB_STACK_SZ (1024)

/**
@brief 
    Defines a process control block structure for maintaining process information\
        for a process.
@var pname
    The current name of a process
@var pcls
    The current class of a process
@var ppri
    The current scheduling priority of a process. The value must be in [0-9]\
        where 0 is the highest priority and increasing values indicate decreasing priority.
@var pstate
    The current execution state of a process.
@var psp
    A pointer to the top of the stack for a process.
*/
struct pcb {
    char pname[MPX_PCB_PROCNAME_SZ];
    enum ProcClass pcls;
    unsigned char ppri;
    enum ProcState pstate;
    void* pbp;
};

/**
@brief
    Allocate memory for a new PCB.
@return
    A non-NULL pointer to a newly allocated PCB on success. NULL on error during allocation\
        or initialization.
*/
struct pcb* pcb_allocate(void);

/**
@brief
    Frees all memory associated with a given PCB, including its stack.
@param pcb
    A pointer to the pcb to free.
@return
    0 on success or otherwise a negative value upon error.
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
    A non-NULL pointer to the created PCB on success, NULL on error during allocation,\
        initialization, or invalid parameters.
*/
struct pcb* pcb_setup(const char* name, enum ProcClass cls, unsigned char pri);

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
