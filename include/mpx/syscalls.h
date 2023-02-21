#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <mpx/device.h>
#include <stddef.h>

/**
@file syscalls.h
@brief Aliases to sysreq and convenience macros for passing buffers to the syscalls.
*/

// macro that gives the determination of the size for constant or literal string arrays
// use only with strings backed by arrays or in conjunction with literals
// good for sys_req to passes the span of a string w/o the automatic null terminator
#define STR_A_SZ(str) (sizeof (str) - 1)
// substitute for the last two paramters to sys_req or similar for the WRITE or READ operations
#define BUF(buf) buf, (sizeof (buf))
#define STR_BUF(str) str, STR_A_SZ(str)

/**
@brief
    Alias for sys_req(WRITE).
@param buffer_in
    An input buffer.
@param buffer_in_sz
    Size of the `buffer_in` buffer.
@return
    A status code corresponding to the result of sys_req(WRITE).
*/
int write(device dev, const void* buffer_in, size_t buffer_in_sz);

/**
@brief
    Alias for sys_req(READ)
@param buffer_inout
    A caller provided buffer to write to.
@param buffer_in_sz
    Size of the `buffer_inout` buffer.
@return
    A status code corresponding to the result of sys_req(WRITE).
*/
int read(device dev, const void* buffer_inout, size_t buffer_inout_sz);

#endif // SYSCALLS_H
