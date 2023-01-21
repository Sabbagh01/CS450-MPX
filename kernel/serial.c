#include <mpx/io.h>
#include <mpx/serial.h>
#include <sys_req.h>
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
    
    // TODO: Finish Backspace key handling (serial echo) and add Delete key handling
    unsigned int b; // general purpose iterator
    unsigned int currsz = 0; // tracked buffer size
    unsigned int i = 0; // position to put next byte
	while ( currsz < len )
    {
        if ( inb (dev + LSR) & 0x01 )
        {
            char c = inb (dev + RBR);
            // check for special buffer/position manipulation keys
            if ( (c == 0x7F) && (i > 0) ) // backspace key
            {
                // manip buffer
                // shift bytes on and after cursor to left to overwrite char i-1
                for (b = 0; b < currsz - i; ++b)
                {
                    buffer[i - 1 + b] = buffer[i + b];
                    outb(dev, buffer[i - 1 + b]);
                }
                buffer[currsz - 1] = '\0'; // clear last character after shift
                --i;
               --currsz;
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
                                outb (dev + RBR, 0x1B);
                                outb (dev + RBR, '[');
                                outb (dev + RBR, 'D');
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
