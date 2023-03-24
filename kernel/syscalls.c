#include <mpx/syscalls.h>

#include <mpx/sys_req.h>


int write(device dev, const void* buffer_in, size_t buffer_in_sz) {
    return sys_req (WRITE, dev, buffer_in, buffer_in_sz);
}

int read(device dev, const void* buffer_inout, size_t buffer_inout_sz) {
    return sys_req (READ, dev, buffer_inout, buffer_inout_sz);
}

int idle() {
    return sys_req (IDLE);
}

int exitret() {
    return sys_req (EXIT);
}
