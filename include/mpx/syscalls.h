#ifndef MPX_SYSCALLS_H
#define MPX_SYSCALLS_H

#include <mpx/device.h>
#include <mpx/bufhelpers.h>
#include <stddef.h>

/**
@file syscalls.h
@brief Aliases to sysreq and convenience macros for passing buffers to the syscalls.
*/

/**
@brief
    Alias for sys_req(WRITE).
@param dev
    Device to read from.
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
    Alias for sys_req(READ).
@param dev
    Device to write to.
@param buffer_inout
    A caller provided buffer to write to.
@param buffer_in_sz
    Size of the `buffer_inout` buffer.
@return
    A status code corresponding to the result of sys_req(WRITE).
*/
int read(device dev, const void* buffer_inout, size_t buffer_inout_sz);

/**
@brief
    Alias for sys_req(IDLE).
@return
    A status code corresponding to the result of sys_req(IDLE).
*/
int idle();

/**
@brief
    Alias for sys_req(EXIT).
@return
    A status code corresponding to the result of sys_req(EXIT).
*/
int exitret();

#endif // MPX_SYSCALLS_H
