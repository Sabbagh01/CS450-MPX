#include <mpx/sys_call.h>

#include <memory.h>

#include <mpx/context.h>
#include <mpx/sys_req.h>
#include <mpx/pcb.h>


void* context_original = NULL;

struct context* sys_call(struct context* context_in)
{
    // note: ctxt_in points to the stack pointer on the stack owned by a running process
    // goal for scheduling out a process is to save the process context on its own
    //     stack, then set pcb->psp to the stack pointer (ESP) so we can dereference
    //     it for scheduling in.
    // goal for scheduling in a process is to get the next pcb, which has a stack pointer
    //     pointing to a context in its stack.
    int op = context_in->eax;
    
    if ((op == READ) || (op == WRITE))
    {
        return (void*)-1;
    }
    struct pcb* runnext = NULL;
    if (op == IDLE)
    {
        // set an original context for the first sys_call IDLE.
        if (context_original == NULL)
        {
            context_original = context_in;
        }
        if (pcb_queues[0].head != NULL)
        {
            // dequeue the next active ready process
            runnext = pcb_queues[0].head->pcb_elem;
            pcb_remove(runnext);
            // set the yielding process' stack pointer to the context to switch to after next run
            pcb_running->pctxt = context_in;
            // enqueue the yielding process into the active ready queue (state unchanged)
            if (pcb_running != NULL)
            {
                pcb_running->pstate = (pcb_running->pstate & ~EXEC_BITS) | READY;
                pcb_insert(pcb_running);
            }
            // set the running pcb to the dequeued one and return its context to switch to
            pcb_running = runnext;
            runnext->pstate = (runnext->pstate & ~EXEC_BITS) | RUNNING;
            return runnext->pctxt;
        }
        else if (context_original != NULL) // no dequeueable processes, continue with first yielded process
        {
            pcb_running = NULL;
            struct context* temp = context_original;
            context_original = NULL;
            return temp;
        }
    }
    if (op == EXIT)
    {
        pcb_free(pcb_running);
        if (pcb_queues[0].head != NULL)
        {
            // dequeue the next active ready process
            runnext = pcb_queues[0].head->pcb_elem;
            pcb_remove(pcb_queues[0].head->pcb_elem);
            // set the running pcb to the dequeued one and return its context to switch to
            pcb_running = runnext;
            return runnext->pctxt;
        }
        else if (context_original != NULL) // no dequeueable processes, load first arrived context
        {
            pcb_running = NULL;
            struct context* temp = context_original;
            context_original = NULL;
            return temp;
        }
    }
    // unrecognized op
    return (void*)-1;
}
