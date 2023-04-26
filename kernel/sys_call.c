#include <mpx/sys_call.h>

#include <memory.h>

#include <mpx/context.h>
#include <mpx/sys_req.h>
#include <mpx/pcb.h>
#include <mpx/serial.h>
#include <mpx/device.h>
#include <mpx/interrupts.h>


void* context_original = NULL;

unsigned char sys_check_io()
{
    unsigned char procs_ready = 0;
    for (size_t i = 0; i < sizeof(serial_dcb_list) / sizeof(struct dcb); ++i)
    {
        if(serial_check_io(serial_dcb_list[i].dev))
        {
            procs_ready = 1;
        }
    }
    return procs_ready;
}

struct context* sys_call(struct context* context_in)
{
    // get requested syscall operation
    int op = context_in->eax;
    device dev;
    void* buffer;
    size_t buffer_sz;

    sys_check_io();

    struct pcb* runnext;
    context_in->eax = 0;
    switch (op)
    {
        // target device:       context_in->ebx
        // given buffer:        context_in->ecx
        // given buffer length: context_in->edx
        case READ:
        {
            if (pcb_running != NULL)
            {
                dev = (device)context_in->ebx;
                buffer = (unsigned char*)context_in->ecx;
                buffer_sz = (size_t)context_in->edx;
                if (serial_schedule_io(dev, buffer, buffer_sz, IO_OP_READ) != 0)
                {
                    // indicate nothing was read via eax (error)
                    // context_in->eax is 0
                    return (void*)0;
                }
                // block process after request
                pcb_running->state.exec = PCB_EXEC_BLOCKED;
                // set the requesting process' stack pointer to the context to switch to after next run
                pcb_running->pctxt = context_in;
                // enqueue the requesting process into the active blocked queue
                pcb_insert(pcb_running);
                if (pcb_queues[0].pcb_head != NULL)
                {
                    // dequeue the next active ready process
                    runnext = pcb_queues[0].pcb_head;
                    pcb_remove(runnext);
                    // set the running pcb to the dequeued one and return its context to switch to
                    pcb_running = runnext;
                    runnext->state.exec = PCB_EXEC_RUNNING;
                    return runnext->pctxt;
                }
                // ready queue empty, so no more processes to execute
                pcb_running = NULL;
                // wait for interrupts to finish operations for any waiting processes
                do
                {
                    sti();
                    __asm__ volatile ("hlt");
                    cli();
                }
                while (!sys_check_io());
                // guaranteed ready pcb at head
                runnext = pcb_queues[0].pcb_head;
                pcb_remove(runnext);
                pcb_running = runnext;
                runnext->state.exec = PCB_EXEC_RUNNING;
            }
            else
            {
                context_in->eax = -1;
                return (void*)0;
            }
        }
        case WRITE:
        {
            if (pcb_running != NULL)
            {
                dev = (device)context_in->ebx;
                buffer = (unsigned char*)context_in->ecx;
                buffer_sz = (size_t)context_in->edx;
                if (serial_schedule_io(dev, buffer, buffer_sz, IO_OP_WRITE) != 0)
                {
                    // indicate nothing was read via eax (error)
                    // context_in->eax is 0
                    return (void*)0;
                }
                // block process after request
                pcb_running->state.exec = PCB_EXEC_BLOCKED;
                // set the requesting process' stack pointer to the context to switch to after next run
                pcb_running->pctxt = context_in;
                // enqueue the requesting process into the active blocked queue
                pcb_insert(pcb_running);
                if (pcb_queues[0].pcb_head != NULL)
                {
                    // dequeue the next active ready process
                    runnext = pcb_queues[0].pcb_head;
                    pcb_remove(runnext);
                    // set the running pcb to the dequeued one and return its context to switch to
                    pcb_running = runnext;
                    runnext->state.exec = PCB_EXEC_RUNNING;
                    return runnext->pctxt;
                }
                // ready queue empty, so no more processes to execute
                pcb_running = NULL;
                // wait for interrupts to finish operations for any waiting processes
                do
                {
                    sti();
                    __asm__ volatile ("hlt");
                    cli();
                }
                while (!sys_check_io());
                // guaranteed ready pcb at head
                runnext = pcb_queues[0].pcb_head;
                pcb_remove(runnext);
                pcb_running = runnext;
                runnext->state.exec = PCB_EXEC_RUNNING;
                return runnext->pctxt;
            }
            else
            {
                context_in->eax = -1;
                return (void*)0;
            }
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
                    pcb_running->state.exec = PCB_EXEC_READY;
                    pcb_insert(pcb_running);
                }
                // set the running pcb to the dequeued one and return its context to switch to
                pcb_running = runnext;
                runnext->state.exec = PCB_EXEC_RUNNING;
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
                runnext->state.exec = PCB_EXEC_RUNNING;
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
    // unrecognized operation, bad request or error
    return (void*)0;
}
