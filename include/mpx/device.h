#ifndef MPX_DEVICES_H
#define MPX_DEVICES_H

#include <stddef.h>
#include <mpx/pcb.h>

typedef enum {
	COM1 = 0x3f8,
	COM2 = 0x2f8,
	COM3 = 0x3e8,
	COM4 = 0x2e8,
} device;

typedef enum io_op {
    IO_READ =  0x00,
    IO_WRITE = 0x01,
} io_op;

struct dcb;

struct iocb
{
    unsigned char op: 1;
    struct pcb* proc;
    struct dcb* dev;
    struct iocb* p_next;
    void* buffer;
    size_t buffer_sz;
};

struct dcb
{
    unsigned char open:  1; // allocation state
    unsigned char idle:  1; // indicates no active operation
    unsigned char op:    1; // current (active) operation, if any
    unsigned char event: 1; // event flag
    struct pcb* curr_proc;
    struct iocb* op_queue;
    void* rbuffer;
    size_t rbuffer_sz;
    size_t rbuffer_pos;
};

#endif
