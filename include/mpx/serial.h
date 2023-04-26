#ifndef MPX_SERIAL_H
#define MPX_SERIAL_H

#include <stddef.h>
#include <mpx/device.h>

/**
 @file mpx/serial.h
 @brief Kernel functions and constants for handling serial I/O
*/

/**
 Initializes devices for user input and output
 @param device A serial port to initialize (COM1, COM2, COM3, or COM4)
 @return 0 on success, non-zero on failure
*/
int serial_init(device dev);

/**
 Writes a buffer to a serial port
 @param device The serial port to output to
 @param buffer A pointer to an array of characters to output
 @param len The number of bytes to write
 @return The number of bytes written
*/
int serial_out(device dev, const char *buffer, size_t len);

/**
 Reads a string from a serial port
 @param device The serial port to read data from
 @param buffer A buffer to write data into as it is read from the serial port
 @param count The maximum number of bytes to read
 @return The number of bytes read on success, a negative number on failure
*/
int serial_poll(device dev, char *buffer, size_t len);

extern struct dcb serial_dcb_list[4];

typedef enum serial_errors
{
    SERIAL_ERR_DEV_NOT_FOUND       =   -1,
    SERIAL_O_ERR_INVALID_EVPTR     = -101,
    SERIAL_O_ERR_INVALID_SPEED     = -102,
    SERIAL_O_ERR_PORT_ALREADY_OPEN = -103,
    SERIAL_C_ERR_PORT_NOT_OPEN     = -201,
    SERIAL_C_ERR_DEV_BUSY          = -204,
    SERIAL_R_ERR_PORT_NOT_OPEN     = -301,
    SERIAL_R_ERR_INVALID_BUFFER    = -302,
    SERIAL_R_ERR_INVALID_BUF_LEN   = -303,
    SERIAL_R_ERR_DEV_BUSY          = -304,
    SERIAL_R_ERR_OUT_OF_MEM        = -305,
    SERIAL_W_ERR_PORT_NOT_OPEN     = -401,
    SERIAL_W_ERR_INVALID_BUFFER    = -402,
    SERIAL_W_ERR_INVALID_BUF_LEN   = -403,
    SERIAL_W_ERR_DEV_BUSY          = -404,
    SERIAL_W_ERR_OUT_OF_MEM        = -405,
    SERIAL_S_ERR_DEV_NOT_FOUND     = -500,
    SERIAL_S_ERR_PORT_NOT_OPEN     = -501,
    SERIAL_S_ERR_INVALID_BUFFER    = -502,
    SERIAL_S_ERR_INVALID_BUF_LEN   = -503,
    SERIAL_S_ERR_DEV_BUSY          = -504,
    SERIAL_S_ERR_OUT_OF_MEM        = -505,
} serial_errors;

/**

*/
int serial_open(device dev, int speed);

/**

*/
int serial_close(device dev);

int serial_check_io(device dev);

int serial_schedule_io(device dev, unsigned char* buffer, size_t buffer_sz,
                       unsigned char io_op);


extern void serial_isr(void*);

/**

*/
void serial_interrupt(void);

#endif
