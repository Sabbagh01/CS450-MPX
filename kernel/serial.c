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
	if (dno == -1 || initialized[dno] == 0) {
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
        if ( inb (dev + LSR) & 0x01 )
        {
            c = inb (dev + RBR);
            // check for special buffer/position manipulation keys
            if ( (c == 0x7F) && (i > 0) ) // backspace key
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
            else if ( c == '~' ) // delete key - mapped to send '~' for some reason
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
    { COM1, NULL, (unsigned char*)&serial_rbuffers[0], SERIAL_RBUFFER_SIZE, 0, 0, 0, 1, 0, 0 },
    { COM2, NULL, (unsigned char*)&serial_rbuffers[1], SERIAL_RBUFFER_SIZE, 0, 0, 0, 1, 0, 0 },
    { COM3, NULL, (unsigned char*)&serial_rbuffers[2], SERIAL_RBUFFER_SIZE, 0, 0, 0, 1, 0, 0 },
    { COM4, NULL, (unsigned char*)&serial_rbuffers[3], SERIAL_RBUFFER_SIZE, 0, 0, 0, 1, 0, 0 },
};

enum serial_errors
{
    SERIAL_ERR_DEV_NOT_FOUND       =   -1,
    SERIAL_O_ERR_INVALID_EVPTR     = -101,
    SERIAL_O_ERR_INVALID_SPEED     = -102,
    SERIAL_O_ERR_PORT_ALREADY_OPEN = -103,
    SERIAL_C_ERR_PORT_NOT_OPEN     = -201,
};

const int serial_supported_baud_rates[] =
{
    110, 150, 300, 600, 1200, 2400, 4800, 9600, 19200,
};

#define SERIAL_IRQ_COM_2_4 (3)
#define SERIAL_IRQ_COM_1_3 (4)

int serial_open(device dev, int speed)
{
	int dno = serial_devno(dev);
	if (dno == -1) {
		return SERIAL_ERR_DEV_NOT_FOUND;
	}
    if (!serial_dcb_list[dno].open)
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
    serial_dcb_list[dno].open = 1;

    switch (dev)
    {
    case COM1:
        if (!serial_dcb_list[serial_devno(COM3)].open)
        {
            idt_install(IRQV_BASE + 4, serial_isr);
        }
        break;
    case COM3:
        if (!serial_dcb_list[serial_devno(COM1)].open)
        {
            idt_install(IRQV_BASE + 4, serial_isr);
        }
        break;
    case COM2:
        if (!serial_dcb_list[serial_devno(COM4)].open)
        {
            idt_install(IRQV_BASE + 3, serial_isr);
        }
        break;
    case COM4:
        if (!serial_dcb_list[serial_devno(COM2)].open)
        {
            idt_install(IRQV_BASE + 3, serial_isr);
        }
        break;
    }
    unsigned int brd = 115200 / speed;
	outb(dev + IER, 0x00);	//disable all serial interrupts
	outb(dev + LCR, 0x80);	//set line control register
	outb(dev + DLL, (char)(brd & 0x00FF));	//set bsd least significant byte
	outb(dev + DLM, (char)(brd >> 8));	//set brd most significant byte
	outb(dev + LCR, 0x03);	//lock divisor; 8bits, no parity, one stop
	outb(dev + FCR, 0xC7);	//enable fifo, clear, 14byte threshold
    cli();
    int mask = inb(PIC1_MASK);
    switch (dev)
    {
    case COM1:
    case COM3:
        mask |= IRQ_BIT(SERIAL_IRQ_COM_1_3);
    case COM2:
    case COM4:
        mask |= IRQ_BIT(SERIAL_IRQ_COM_2_4);
    }
    outb(PIC1_MASK, mask);
    sti();
    outb(dev + MCR, 0x08);	//enable device interrupts, no rts/dsr
    outb(dev + IER, 0x01); //enable input ready interrupts
	(void)inb(dev);		//read bit to reset port
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

    serial_dcb_list[dno].open = 0;

    switch (dev)
    {
    case COM1:
        if (serial_dcb_list[serial_devno(COM3)].open)
        {
            goto skip_pic_disable;
        }
        break;
    case COM3:
        if (serial_dcb_list[serial_devno(COM1)].open)
        {
            goto skip_pic_disable;
        }
        break;
    case COM2:
        if (serial_dcb_list[serial_devno(COM4)].open)
        {
            goto skip_pic_disable;
        }
        break;
    case COM4:
        if (serial_dcb_list[serial_devno(COM2)].open)
        {
            goto skip_pic_disable;
        }
        break;
    }
    cli();
    int mask = inb(PIC1_MASK);
    switch (dev)
    {
    case COM1:
    case COM3:
        mask |= IRQ_BIT(SERIAL_IRQ_COM_1_3);
    case COM2:
    case COM4:
        mask |= IRQ_BIT(SERIAL_IRQ_COM_2_4);
    }
    outb(PIC1_MASK, mask);
    sti();

    skip_pic_disable: ;
    outb(dev + IER, 0x00); // disable all serial interrupts
    outb(dev + MCR, 0x00); // disable all device interrupts

    return 0;
}

int serial_read(device dev, char* buf, size_t len);

int serial_write(device dev, char* buf, size_t len);

void serial_schedule_io(device dev, struct pcb* pcb, void* buffer,
                        size_t buffer_sz, unsigned char io_op)
{
    return;
}

#define SERIAL_IIR_

void serial_interrupt(void) {
    cli();
    // get IRQ to identify serial device(s)
    // command to read ISR
    outb(PIC1_CMD, PIC_READ_ISR);
    unsigned char irq = inb(PIC1_CMD);
    struct dcb* dcb_select;
    unsigned char serial_iir;
    switch (irq)
    {
    // select specific device via bit 0 of IIR in associated serial devices 
    // COM2 or COM4
    case IRQ_BIT(SERIAL_IRQ_COM_2_4):
        // check for COM2 interrupt
        serial_iir = inb(COM2 + IIR);
        if (serial_iir & 0x01)
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
    // COM1 or COM3
    case IRQ_BIT(SERIAL_IRQ_COM_1_3):
        // check for COM1 interrupt
        serial_iir = inb(COM1 + IIR);
        if (serial_iir & 0x01)
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

    // check that selected device is open, if not, ignore interrupt and return
    if (!dcb_select->open)
    {
        goto handler_exit;
    }

    // check interrupt type for the device
    switch (serial_iir & 0x06)
    {
    case (0 << 1): // Modem Status
        // TODO: Add second-level handler
        break;
    case (1 << 1): // Output
        serial_output_interrupt(dcb_select);
        break;
    case (2 << 1): // Input
        serial_input_interrupt(dcb_select);
        break;
    case (3 << 1): // Line Status
        // TODO: Add second-level handler
        break;
    }
    
    handler_exit: ;
    outb(PIC1_CMD, PIC_EOI);
    sti();
    return;
}

void serial_input_interrupt(struct dcb* dcb)
{
    unsigned char byte = inb(dcb->dev);
    if (dcb->op != IO_OP_READ)
    {
        if (dcb->rbuffer_idx_read == dcb->rbuffer_idx_write)
        {
            return;
        }
        dcb->rbuffer[dcb->rbuffer_idx_write] = byte;
        ++dcb->rbuffer_idx_write;
        if (dcb->rbuffer_idx_write == dcb->rbuffer_sz)
        {
            dcb->rbuffer_idx_write = 0;
        }
    }
    else
    {
        // alias
        struct iocb* iocb_rq = dcb->iocb_queue_head;

        iocb_rq->buffer[iocb_rq->buffer_idx] = byte;
        ++iocb_rq->buffer_idx;
        if ((iocb_rq->buffer_idx == iocb_rq->buffer_sz) || (byte == '\n'))
        {
            dcb->idle = 1;
            dcb->event = 1;
        }
    }
    return;
}

void serial_output_interrupt(struct dcb* dcb)
{
    if (dcb->op != IO_OP_WRITE)
    {
        return;
    }
    else
    {
        // alias
        struct iocb* iocb_rq = dcb->iocb_queue_head;

        unsigned char next_byte = iocb_rq->buffer[iocb_rq->buffer_idx];
        outb(dcb->dev, next_byte);
        ++iocb_rq->buffer_idx;
        if (iocb_rq->buffer_idx == iocb_rq->buffer_sz)
        {
            dcb->idle = 1;
            dcb->event = 1;
            // clear write-out interrupts
            next_byte = inb(dcb->dev + IER);
            outb(dcb->dev + IER, next_byte & ~(1 << 1));
        }
    }
    return;
}
