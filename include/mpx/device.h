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

struct iocb
{
    struct iocb* p_next;
    struct pcb* pcb_rq;
    unsigned char* buffer;
    size_t buffer_sz;
    unsigned char io_op: 1;
};

struct dcb
{
    device dev;
    struct iocb* iocb_queue_head; // if pcb_rq is NULL, the dcb is idle and other state in iocb should be ignored
    struct iocb* iocb_queue_tail;
    unsigned char* rbuffer;
    size_t rbuffer_sz;
    size_t rbuffer_idx_begin; // read index (to read from next)
    size_t rbuffer_idx_end; // write index (to write to next) [if begin == end, rbuffer must be empty]
    size_t buffer_idx; // indicates progress (how much has been read from, written to the iocb buffer)
    unsigned char open:  1; // initialization state
    unsigned char event: 1; // event flag
};

#endif
