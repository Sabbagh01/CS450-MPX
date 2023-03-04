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
    int op;
    __asm__ volatile("mov %0, %%eax" : "=r"(op));
    
    // set an original context for the first sys_call entry.
    if (context_original == NULL)
    {
        context_original = context_in;
    }
    
    if ((op == READ) || (op == WRITE))
    {
        return (void*)-1;
    }
    struct pcb* runnext = NULL;
    if (op == IDLE)
    {
        if (pcb_queues[0].head != NULL)
        {
            // dequeue the next active ready process
            runnext = pcb_queues[0].head->pcb_elem;
            pcb_remove(pcb_queues[0].head->pcb_elem);
            // set the yielding process' stack pointer to the context to switch to after next run
            pcb_running->psp = context_in;
            // enqueue the yielding process into the active ready queue (state unchanged)
            pcb_insert(pcb_running);
            // set the running pcb to the dequeued one and return its context to switch to
            pcb_running = runnext;
            return runnext->psp;
        }
        else // no dequeueable processes, continue with yielded process
        {
            return context_in;
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
            return runnext->psp;
        }
        else // no dequeueable processes, load first arrived context
        {
            return context_original;
        }
    }
    // unrecognized op
    return (void*)-1;
}
