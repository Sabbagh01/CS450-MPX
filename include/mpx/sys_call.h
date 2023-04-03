#ifndef MPX_SYS_CALL_H
#define MPX_SYS_CALL_H

#include <mpx/context.h>

/**
 @brief
    A syscall handler called as part of the syscall interrupt service routine.
 @param context_in
    A pointer to a context pushed as part of interrupt servicing from sys_call_isr.
 @return
    A pointer to a different context to switch and return to, or NULL to indicate 
    returning back to the originally pushed context.
*/
struct context* sys_call(struct context* context_in);

#endif // MPX_SYS_CALL_H
