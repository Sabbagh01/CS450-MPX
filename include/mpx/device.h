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
    IO_OP_READ =  0x00,
    IO_OP_WRITE = 0x01,
} io_op;

struct dcb;

struct iocb
{
    struct iocb* p_next;
    struct pcb* pcb_req;
    void* buffer;
    size_t buffer_sz;
    unsigned char io_op: 1;
};

struct dcb
{
    struct iocb* iocb_queue_head;
    void* rbuffer;
    size_t rbuffer_sz;
    size_t rbuffer_idx_read;  // read index (to read from next)
    size_t rbuffer_idx_write; // write index (to write to next)
    unsigned char open:  1; // allocation state
    unsigned char idle:  1; // indicates no active operation
    unsigned char op:    1; // current (active) operation, if any
    unsigned char event: 1; // event flag
};

#endif
