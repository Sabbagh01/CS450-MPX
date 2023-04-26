#include <mpx/io.h>
#include <mpx/gdt.h>
#include <mpx/serial.h>
#include <mpx/interrupts.h>
#include <memory.h>
#include <mpx/sys_req.h>
#include <ctype.h>

enum uart_registers {
	RBR = 0,	// Receive Buffer
	THR = 0,	// Transmitter Holding
	DLL = 0,	// Divisor Latch LSB
	IER = 1,	// Interrupt Enable
	DLM = 1,	// Divisor Latch MSB
	IIR = 2,	// Interrupt Identification
	FCR = 2,	// FIFO Control
	LCR = 3,	// Line Control
	MCR = 4,	// Modem Control
	LSR = 5,	// Line Status
	MSR = 6,	// Modem Status
	SCR = 7,	// Scratch
};

static int initialized[4] = { 0 };

static int serial_devno(device dev)
{
	switch (dev) {
	case COM1: return 0;
	case COM2: return 1;
	case COM3: return 2;
	case COM4: return 3;
	}
	return -1;
}

int serial_init(device dev)
{
	int dno = serial_devno(dev);
	if (dno == -1) {
		return -1;
	}
	outb(dev + IER, 0x00);	//disable interrupts
	outb(dev + LCR, 0x80);	//set line control register
	outb(dev + DLL, 115200 / 9600);	//set bsd least sig bit
	outb(dev + DLM, 0x00);	//brd most significant bit
	outb(dev + LCR, 0x03);	//lock divisor; 8bits, no parity, one stop
	outb(dev + FCR, 0xC7);	//enable fifo, clear, 14byte threshold
	outb(dev + MCR, 0x0B);	//enable interrupts, rts/dsr set
	(void)inb(dev);		//read bit to reset port
	initialized[dno] = 1;
	return 0;
}

int serial_out(device dev, const char *buffer, size_t len)
{
	int dno = serial_devno(dev);
	if (dno == -1)
    {
		return -1;
	}
    if (!initialized[dno])
    {
        return -1;
    }
	for (size_t i = 0; i < len; i++) {
		outb(dev, buffer[i]);
	}
	return (int)len;
}

int serial_poll(device dev, char *buffer, size_t len)
{
    int dno = serial_devno(dev);
	if (dno == -1 || initialized[dno] == 0)
    {
		return -1;
	}
    --len; // leave a NUL terminator on the end.
    buffer[len] = '\0';
    unsigned int b; // general purpose iterator
    unsigned int currsz = 0; // tracked buffer size
    unsigned int i = 0; // position to put next byte
    unsigned char c;
	while ( currsz < len )
    {
        if ( inb(dev + LSR) & 0x01 )
        {
            c = inb (dev + RBR);
            // check for special buffer/position manipulation keys
            if ( (c == 0x08) && (i > 0) ) // backspace key
            {
                // manip buffer
                // shift bytes on and after cursor to left to overwrite char i-1
                for (b = 0; b < currsz - i; ++b)
                {
                    buffer[i - 1 + b] = buffer[i + b];
                }
                --i;
                --currsz;
                buffer[currsz] = '\0'; // clear last character after shift
                outb (dev + RBR, '\r');
                for (b = 0; b < currsz; ++b)
                {
                    outb (dev + RBR, buffer[b]);
                }
                outb (dev + RBR, ' ');
                for (b = 0; b < currsz - i + 1; ++b)
                {
                    outb (dev + RBR, 0x1B);
                    outb (dev + RBR, '[');
                    outb (dev + RBR, 'D');
                }
            }
            else if (c == 0x7F) // delete key
            {
                if (i < currsz)
                {
                    // manip buffer
                    // shift bytes on and after cursor to the left to overwrite char i
                    for (b = 0; b < currsz - i; ++b)
                    {
                        buffer[i + b] = buffer[(i + 1) + b];
                    }
                    --currsz;
                    buffer[currsz] = '\0'; // clear last character after shift
                    outb (dev + RBR, '\r');
                    for (b = 0; b < currsz; ++b)
                    {
                        outb (dev + RBR, buffer[b]);
                    }
                    outb (dev + RBR, ' ');
                    for (b = 0; b < currsz - i + 1; ++b)
                    {
                        outb (dev + RBR, 0x1B);
                        outb (dev + RBR, '[');
                        outb (dev + RBR, 'D');
                    }
                }
            }
            else if ( c == 0x1B ) // escape sequence
            {
                c = inb (dev + RBR);
                if ( c == '[' ) // cursor sequence
                {
                    c = inb (dev + RBR);
                    switch (c)
                    {
                        case 'D': // cursor to left
                            if (i > 0) // prevent cursor underflowing
                            {
                                --i;
                                // outb (dev + RBR, 0x1B);
                                // outb (dev + RBR, '[');
                                // outb (dev + RBR, 'D');
                                outb(dev, '\b');
                            }
                            break;
                        case 'C': // cursor to right
                            if (i < currsz) // prevent cursor going over the current text
                            {
                                ++i;
                                outb (dev + RBR, 0x1B);
                                outb (dev + RBR, '[');
                                outb (dev + RBR, 'C');
                            }
                            break;
                    }
                }
            }
            else if ( (c >= '!' && c <= '~') || isspace (c) ) // handle insertion of symbolic characters (includes alphanum)
            {
                if ( (c == '\n') || (c == '\r') )
                {
                    break; // poll breakpoint (ENTER recieved)
                }
                for (b = currsz - i; b > 0; --b)
                {
                    buffer[i + b] = buffer[i + b - 1];
                }
                buffer[i] = c;
                ++i;
                ++currsz;
                outb (dev + RBR, '\r');
                for (b = 0; b < currsz; ++b)
                {
                    outb(dev + RBR, buffer[b]);
                }
                for (b = 0; b < currsz - i; ++b)
                {
                    outb (dev + RBR, 0x1B);
                    outb (dev + RBR, '[');
                    outb (dev + RBR, 'D');
                }
            }
        }
    }
    return currsz;
}

#define SERIAL_RBUFFER_SIZE (32)
unsigned char serial_rbuffers[4][SERIAL_RBUFFER_SIZE];
// serial ports start closed
struct dcb serial_dcb_list[4] = {
    { COM1, NULL, NULL, (unsigned char*)&serial_rbuffers[0], SERIAL_RBUFFER_SIZE, 0, 0, 0, 0, 0 },
    { COM2, NULL, NULL, (unsigned char*)&serial_rbuffers[1], SERIAL_RBUFFER_SIZE, 0, 0, 0, 0, 0 },
    { COM3, NULL, NULL, (unsigned char*)&serial_rbuffers[2], SERIAL_RBUFFER_SIZE, 0, 0, 0, 0, 0 },
    { COM4, NULL, NULL, (unsigned char*)&serial_rbuffers[3], SERIAL_RBUFFER_SIZE, 0, 0, 0, 0, 0 },
};

#define SERIAL_IRQ_COM_2_4 (3)
#define SERIAL_IRQ_COM_1_3 (4)

int serial_open(device dev, int speed)
{
    static const int serial_supported_baud_rates[] =
    {
        110, 150, 300, 600, 1200, 2400, 4800, 9600, 19200,
    };

	int dno = serial_devno(dev);
	if (dno == -1)
    {
		return SERIAL_ERR_DEV_NOT_FOUND;
	}
    if (serial_dcb_list[dno].open)
    {
        return SERIAL_O_ERR_PORT_ALREADY_OPEN;
    }
    for (size_t i = 0; i < sizeof(serial_supported_baud_rates); ++i)
    {
        if (serial_supported_baud_rates[i] == speed)
        {
            goto baud_rate_matched;
        }
    }
    return SERIAL_O_ERR_INVALID_SPEED;
    
    baud_rate_matched: ;
    // proceed to setup dcb
    // the commented statements are not needed, as those fields should remain constant with set buffers
    //    serial_dcb_list[dno].rbuffer = &serial_rbuffers[dno];
    //    serial_dcb_list[dno].rbuffer_sz = SERIAL_RBUFFER_SIZE;
    // ensure ring buffer is reset
    serial_dcb_list[dno].rbuffer_idx_begin = 0;
    serial_dcb_list[dno].rbuffer_idx_end = 0;
    serial_dcb_list[dno].open = 1;
    serial_dcb_list[dno].event = 0;

    switch (dev)
    {
    case COM1:
    {
        if (!serial_dcb_list[serial_devno(COM3)].open)
        {
            idt_install(IRQV_BASE + SERIAL_IRQ_COM_1_3, serial_isr);
        }
        break;
    }
    case COM3:
    {
        if (!serial_dcb_list[serial_devno(COM1)].open)
        {
            idt_install(IRQV_BASE + SERIAL_IRQ_COM_1_3, serial_isr);
        }
        break;
    }
    case COM2:
    {
        if (!serial_dcb_list[serial_devno(COM4)].open)
        {
            idt_install(IRQV_BASE + SERIAL_IRQ_COM_2_4, serial_isr);
        }
        break;
    }
    case COM4:
    {
        if (!serial_dcb_list[serial_devno(COM2)].open)
        {
            idt_install(IRQV_BASE + SERIAL_IRQ_COM_2_4, serial_isr);
        }
        break;
    }
    }
    unsigned int brd = 115200 / speed;
	outb(dev + IER, 0x00);	//disable all serial interrupts
	outb(dev + LCR, 0x80);	//set line control register
	outb(dev + DLL, (char)(brd));	    //set brd least significant byte
	outb(dev + DLM, (char)(brd >> 8));	//set brd most significant byte
	outb(dev + LCR, 0x03);	//lock divisor; 8bits, no parity, one stop
	outb(dev + FCR, 0xC7);	//enable fifo, clear, 14byte threshold
    cli();
    int mask = inb(PIC_1_MASK);
    switch (dev)
    {
    case COM1:
    case COM3:
    {
        mask &= ~IRQ_BIT(SERIAL_IRQ_COM_1_3);
    }
    case COM2:
    case COM4:
    {
        mask &= ~IRQ_BIT(SERIAL_IRQ_COM_2_4);
    }
    }
    outb(PIC_1_MASK, mask);
    outb(dev + MCR, (1 << 3));	// only enable device interrupts, set no rts/dsr
    outb(dev + IER, (1 << 0)); // only enable serial input data received interrupts
	inb(dev); // read byte to reset port
    sti();
	return 0;
}

int serial_close(device dev)
{
    int dno = serial_devno(dev);
    if (dno == -1)
    {
        return SERIAL_ERR_DEV_NOT_FOUND;
    }
    if (!serial_dcb_list[dno].open)
    {
        return SERIAL_C_ERR_PORT_NOT_OPEN;
    }
    // ensure that there are no operations currently executing on the device
    if (serial_dcb_list[dno].iocb_queue_head != NULL)
    {
        return SERIAL_C_ERR_DEV_BUSY;
    }
    // check if any PIC interrupts can be masked without affecting other open devices
    switch (dev)
    {
    case COM1:
    {
        if (serial_dcb_list[serial_devno(COM3)].open)
        {
            goto skip_pic_disable;
        }
        break;
    }
    case COM3:
    {
        if (serial_dcb_list[serial_devno(COM1)].open)
        {
            goto skip_pic_disable;
        }
        break;
    }
    case COM2:
    {
        if (serial_dcb_list[serial_devno(COM4)].open)
        {
            goto skip_pic_disable;
        }
        break;
    }
    case COM4:
    {
        if (serial_dcb_list[serial_devno(COM2)].open)
        {
            goto skip_pic_disable;
        }
        break;
    }
    }
    // if corresponding PIC bit can be masked (no other open devices mapped to it), do so.
    cli();
    int mask = inb(PIC_1_MASK);
    switch (dev)
    {
    case COM1:
    case COM3:
    {
        mask |= IRQ_BIT(SERIAL_IRQ_COM_1_3);
    }
    case COM2:
    case COM4:
    {
        mask |= IRQ_BIT(SERIAL_IRQ_COM_2_4);
    }
    }
    outb(PIC_1_MASK, mask);
    sti();
    skip_pic_disable: ;
    outb(dev + IER, 0x00); // disable all serial interrupt types
    outb(dev + MCR, 0x00); // disable serial device raising interrupts

    // set selected device to closed
    serial_dcb_list[dno].open = 0;
    return 0;
}

int serial_read(device dev, char* buf, size_t len)
{
    if (buf == NULL)
    {
        return SERIAL_R_ERR_INVALID_BUFFER;
    }
    if (len == 0)
    {
        return SERIAL_R_ERR_INVALID_BUF_LEN;
    }
    int devno = serial_devno(dev);
    if (devno == -1)
    {
        return SERIAL_ERR_DEV_NOT_FOUND;
    }
    struct dcb* dcb_select = &serial_dcb_list[devno];
    if (!dcb_select->open)
    {
        return SERIAL_R_ERR_PORT_NOT_OPEN;
    }
    if (dcb_select->iocb_queue_head != NULL)
    {
        return SERIAL_R_ERR_DEV_BUSY;
    }

    struct iocb* iocb_new = (struct iocb*) sys_alloc_mem(sizeof(struct iocb));
    if (iocb_new == NULL)
    {
        return SERIAL_R_ERR_OUT_OF_MEM;
    }
    dcb_select->iocb_queue_head = iocb_new;
    dcb_select->iocb_queue_tail = iocb_new;
    iocb_new->p_next = NULL;
    iocb_new->pcb_rq = pcb_running;
    iocb_new->buffer = (unsigned char*) buf;
    iocb_new->buffer_sz = len;
    iocb_new->io_op = IO_OP_READ;

    size_t buf_idx = 0;
    while ((dcb_select->rbuffer_idx_begin < dcb_select->rbuffer_idx_end)
           && (buf_idx < len))
    {
        buf[buf_idx] = dcb_select->rbuffer[dcb_select->rbuffer_idx_begin];
        ++dcb_select->rbuffer_idx_begin;
        // loop the ring buffer 'begin' index if needed
        if (dcb_select->rbuffer_idx_begin == dcb_select->rbuffer_sz)
        {
            dcb_select->rbuffer_idx_begin = 0;
        }
        if (buf[buf_idx] == '\r')
        {
            ++buf_idx;
            goto read_complete;
        }
        ++buf_idx;
    }
    if (buf_idx < len)
    {
        dcb_select->buffer_idx = buf_idx;
        return 0;
    }
    // buf_idx == len will cause fallthrough to here, as it indicates completion
    read_complete: ;
    dcb_select->buffer_idx = buf_idx;
    return 0;
}

int serial_write(device dev, char* buf, size_t len)
{
    if (buf == NULL)
    {
        return SERIAL_W_ERR_INVALID_BUFFER;
    }
    if (len == 0)
    {
        return SERIAL_W_ERR_INVALID_BUF_LEN;
    }
    int devno = serial_devno(dev);
    if (devno == -1)
    {
        return SERIAL_ERR_DEV_NOT_FOUND;
    }
    struct dcb* dcb_select = &serial_dcb_list[devno];
    if (!dcb_select->open)
    {
        return SERIAL_W_ERR_PORT_NOT_OPEN;
    }
    if (dcb_select->iocb_queue_head != NULL)
    {
        return SERIAL_W_ERR_DEV_BUSY;
    }

    struct iocb* iocb_new = (struct iocb*) sys_alloc_mem(sizeof(struct iocb));
    if (iocb_new == NULL)
    {
        return SERIAL_W_ERR_OUT_OF_MEM;
    }
    dcb_select->iocb_queue_head = iocb_new;
    dcb_select->iocb_queue_tail = iocb_new;
    iocb_new->p_next = NULL;
    iocb_new->pcb_rq = pcb_running;
    iocb_new->buffer = (unsigned char*) buf;
    iocb_new->buffer_sz = len;
    iocb_new->io_op = IO_OP_WRITE;
    dcb_select->buffer_idx = 0;

    unsigned char next_byte = buf[0];
    outb(dcb_select->dev, next_byte);
    int ier = inb(dev + IER);
    outb(dev + IER, (ier | (1 << 1)));
    return 0;
}

int serial_check_io(device dev)
{
    int dno = serial_devno(dev);
    if (dno == -1)
    {
        return -1;
    }
    struct dcb* dcb_select = &serial_dcb_list[dno];
    if (!dcb_select->open)
    {
        return 0;
    }
    if (dcb_select->event)
    {
        // queue a pcb if it has an event ready
        // alias
        struct pcb* pcb_hasevent = dcb_select->iocb_queue_head->pcb_rq;
        pcb_remove(pcb_hasevent);

        pcb_hasevent->state.exec = PCB_EXEC_READY;
        pcb_hasevent->pctxt->eax = dcb_select->buffer_idx;

        pcb_insert(pcb_hasevent);
        dcb_select->event = 0;

        // free iocb from active operation and proceed to the next, if any
        struct iocb* iocb_next = dcb_select->iocb_queue_head->p_next;
        sys_free_mem(dcb_select->iocb_queue_head);
        // only one operation in the list
        if (dcb_select->iocb_queue_head == dcb_select->iocb_queue_tail)
        {
            dcb_select->iocb_queue_tail = iocb_next;
        }
        dcb_select->iocb_queue_head = iocb_next;
        // if there is a next operation, configure accordingly
        if (iocb_next != NULL)
        {
            switch (iocb_next->io_op)
            {
            case IO_OP_READ:
            {
                size_t buf_idx = 0;
                while ((dcb_select->rbuffer_idx_begin < dcb_select->rbuffer_idx_end)
                       && (buf_idx < iocb_next->buffer_sz))
                {
                    iocb_next->buffer[buf_idx] = dcb_select->rbuffer[dcb_select->rbuffer_idx_begin];
                    ++dcb_select->rbuffer_idx_begin;
                    // loop the ring buffer 'begin' index if needed
                    if (dcb_select->rbuffer_idx_begin == dcb_select->rbuffer_sz)
                    {
                        dcb_select->rbuffer_idx_begin = 0;
                    }
                    if (iocb_next->buffer[buf_idx] == '\r')
                    {
                        ++buf_idx;
                        goto read_complete;
                    }
                    ++buf_idx;
                }
                if (buf_idx < iocb_next->buffer_sz)
                {
                    dcb_select->buffer_idx = buf_idx;
                    break;
                }
                // buf_idx == len will cause fallthrough to here, as it indicates completion
                read_complete: ;
                dcb_select->buffer_idx = buf_idx;
                break;
            }
            case IO_OP_WRITE:
            {
                outb(serial_dcb_list[dno].dev, iocb_next->buffer[0]);
                int ier = inb(dev + IER);
                outb(dev + IER, (ier | (1 << 1)));
                break;
            }
            }
        }
        return 1;
    }
    return 0;
}

int serial_schedule_io(device dev, unsigned char* buffer, size_t buffer_sz,
                       unsigned char io_op)
{
    if (buffer == NULL)
    {
        return SERIAL_S_ERR_INVALID_BUFFER;
    }
    if (buffer_sz == 0)
    {
        return SERIAL_S_ERR_INVALID_BUF_LEN;
    }
    int devno = serial_devno(dev);
    if (devno == -1)
    {
        return SERIAL_S_ERR_DEV_NOT_FOUND;
    }
    struct dcb* dcb_select = &serial_dcb_list[devno];
    if (!dcb_select->open)
    {
        return SERIAL_S_ERR_PORT_NOT_OPEN;
    }
    // check for no queued operations (device is idle)
    if (dcb_select->iocb_queue_head == NULL)
    {
        int ret;
        switch (io_op)
        {
        case IO_OP_READ:
        {
            ret = serial_read(dev, (char*)buffer, buffer_sz);
            if (ret != 0)
            {
                switch (ret)
                {
                case SERIAL_R_ERR_DEV_BUSY:
                {
                    return SERIAL_S_ERR_DEV_BUSY;
                }
                case SERIAL_R_ERR_OUT_OF_MEM:
                {
                    return SERIAL_S_ERR_OUT_OF_MEM;
                }
                }
            }
            break;
        }
        case IO_OP_WRITE:
        {
            int ret = serial_write(dev, (char*)buffer, buffer_sz);
            if (ret != 0)
            {
                switch (ret)
                {
                case SERIAL_W_ERR_DEV_BUSY:
                {
                    return SERIAL_S_ERR_DEV_BUSY;
                }
                case SERIAL_W_ERR_OUT_OF_MEM:
                {
                    return SERIAL_S_ERR_OUT_OF_MEM;
                }
                }
            }
            break;
        }
        }
    }
    else // selected device is not idle
    {
        // queue an I/O operation on the selected device
        struct iocb* iocb_new = (struct iocb*) sys_alloc_mem(sizeof(struct iocb));
        if (iocb_new == NULL)
        {
            return SERIAL_S_ERR_OUT_OF_MEM;
        }
        iocb_new->p_next = NULL;
        iocb_new->pcb_rq = pcb_running;
        iocb_new->buffer = buffer;
        iocb_new->buffer_sz = buffer_sz;
        iocb_new->io_op = io_op;

        dcb_select->iocb_queue_tail->p_next = iocb_new;
        dcb_select->iocb_queue_tail = iocb_new;
    }
    return 0;
}

void serial_input_interrupt(struct dcb* dcb)
{
    unsigned char byte = inb(dcb->dev);
    if (dcb->iocb_queue_head->io_op != IO_OP_READ)
    {
        // store input byte in ring buffer for next READ request to initially copy
        dcb->rbuffer[dcb->rbuffer_idx_end] = byte;
        // shift ring buffer begin index if at capacity (having wrote on top of beginning index)
        if (dcb->rbuffer_idx_end == dcb->rbuffer_idx_begin)
        {
            ++dcb->rbuffer_idx_begin;
            if (dcb->rbuffer_idx_begin == dcb->rbuffer_sz)
            {
                dcb->rbuffer_idx_begin = 0;
            }
        }
        ++dcb->rbuffer_idx_end;
        if (dcb->rbuffer_idx_end == dcb->rbuffer_sz)
        {
            dcb->rbuffer_idx_end = 0;
        }
    }
    else if (!dcb->event)
    {
        // alias
        struct iocb* iocb_rq = dcb->iocb_queue_head;

        iocb_rq->buffer[dcb->buffer_idx] = byte;
        ++dcb->buffer_idx;
        if ((dcb->buffer_idx == iocb_rq->buffer_sz) || (byte == '\r'))
        {
            dcb->event = 1;
        }
    }
    return;
}

void serial_output_interrupt(struct dcb* dcb)
{
    if (!dcb->event)
    {
        if (dcb->iocb_queue_head->io_op == IO_OP_WRITE)
        {
            // alias
            struct iocb* iocb_rq = dcb->iocb_queue_head;
            // interrupt confirms byte was sent, so increment here
            ++dcb->buffer_idx;

            // check for write completion
            if (dcb->buffer_idx == iocb_rq->buffer_sz)
            {
                dcb->event = 1;
                // clear write-out interrupts
                unsigned char ier = inb(dcb->dev + IER);
                outb(dcb->dev + IER, ier & ~(1 << 1));
            }
            else // writing is still not done
            {
                unsigned char next_byte = iocb_rq->buffer[dcb->buffer_idx];
                outb(dcb->dev, next_byte);
            }
        }
    }
    return;
}

void serial_interrupt(void)
{
    // get IRQ to identify serial device(s)
    // command to read ISR
    outb(PIC_1_CMD, PIC_READ_ISR);
    unsigned char irq = inb(PIC_1_CMD);
    struct dcb* dcb_select;
    unsigned char serial_iir;
    switch (irq)
    {
    // select specific device via bit 0 of IIR in associated serial devices 
    // COM2 or COM4
    case IRQ_BIT(SERIAL_IRQ_COM_2_4):
    {
        // check for COM2 interrupt
        serial_iir = inb(COM2 + IIR);
        if ((serial_iir & 0x01) == 0)
        {
            dcb_select = &serial_dcb_list[serial_devno(COM2)];
            break;
        }
        else // COM4 interrupt
        {
            serial_iir = inb(COM4 + IIR);
            dcb_select = &serial_dcb_list[serial_devno(COM4)];
        }
        break;
    }
    // COM1 or COM3
    case IRQ_BIT(SERIAL_IRQ_COM_1_3):
    {
        // check for COM1 interrupt
        serial_iir = inb(COM1 + IIR);
        if ((serial_iir & 0x01) == 0)
        {
            dcb_select = &serial_dcb_list[serial_devno(COM1)];
            break;
        }
        else // COM3 interrupt
        {
            serial_iir = inb(COM3 + IIR);
            dcb_select = &serial_dcb_list[serial_devno(COM3)];
        }
        break;
    }
    }

    // check that selected device is open, if not, ignore interrupt and return
    if (!dcb_select->open)
    {
        goto handler_exit;
    }

    // check interrupt type for the device and execute second-level handlers
    switch (serial_iir & 0x06)
    {
    case (0 << 1): // Modem Status
    {
        inb(dcb_select->dev + MSR);
        break;
    }
    case (1 << 1): // Output
    {
        serial_output_interrupt(dcb_select);
        break;
    }
    case (2 << 1): // Input
    {
        serial_input_interrupt(dcb_select);
        break;
    }
    case (3 << 1): // Line Status
    {
        // simply read and discard
        inb(dcb_select->dev + LSR);
        break;
    }
    }
    
    handler_exit: ;
    outb(PIC_1_CMD, PIC_EOI);
    return;
}

