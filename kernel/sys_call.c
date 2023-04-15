#include <mpx/sys_call.h>

#include <memory.h>

#include <mpx/context.h>
#include <mpx/sys_req.h>
#include <mpx/pcb.h>
#include <mpx/device.h>


void* context_original = NULL;

struct context* sys_call(struct context* context_in)
{
    // get requested syscall operation
    int op = context_in->eax;
    
    struct pcb* runnext;
    context_in->eax = 0;
    switch (op)
    {
        // target device:       context_in->ebx
        // given buffer:        context_in->ecx
        // given buffer length: context_in->edx
        case READ:
        case WRITE:
        {
            context_in->eax = -1;
            return (void*)0;
        }

        // note: ctxt_in points to the stack pointer on the stack owned by a running process
        // goal for scheduling out a process is to save the process context on its own
        //     stack, then set pcb->psp to the stack pointer (ESP) so we can dereference
        //     it for scheduling in.
        // goal for scheduling in a process is to get the next pcb, which has a stack pointer
        //     pointing to a context in its stack.
        case IDLE:
        {
            // set an original context for the first sys_call IDLE.
            if (context_original == NULL)
            {
                context_original = context_in;
            }
            // check for any ready processes
            if (pcb_queues[0].pcb_head != NULL)
            {
                // dequeue the next active ready process
                runnext = pcb_queues[0].pcb_head;
                pcb_remove(runnext);
                // enqueue the yielding process (if any) into the active ready queue (state unchanged)
                if (pcb_running != NULL)
                {
                    // set the yielding process' stack pointer to the context to switch to after next run
                    pcb_running->pctxt = context_in;
                    pcb_running->state.exec = READY;
                    pcb_insert(pcb_running);
                }
                // set the running pcb to the dequeued one and return its context to switch to
                pcb_running = runnext;
                runnext->state.exec = RUNNING;
                return runnext->pctxt;
            }
            // continue to original context in the absence of processes
            pcb_running = NULL;
            struct context* temp = context_original;
            context_original = NULL;
            return temp;
        }

        case EXIT:
        {
            pcb_free(pcb_running);
            if (pcb_queues[0].pcb_head != NULL)
            {
                // dequeue the next active ready process
                runnext = pcb_queues[0].pcb_head;
                pcb_remove(pcb_queues[0].pcb_head);
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
            // this case should never be reached in normal operation and state
            return (void*)0;
        }
    }
    // unrecognized operation request or bad request
    return (void*)0;
}
