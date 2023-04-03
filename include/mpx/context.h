#ifndef MPX_CONTEXT_H
#define MPX_CONTEXT_H

#include <stdint.h>


/**
 @struct context
 @brief
    The structure of a context used by sys_call_isr and syscall handlers for
    pushing and popping contexts. Fields correspond to similarly named registers.
*/
struct context
{
    uint16_t gs,
             __pad1,
             fs,
             __pad2,
             es,
             __pad3,
             ds,
             __pad4,
             ss,
             __pad5;
    uint32_t edi,
             esi,
             ebp,
             esp,
             ebx,
             edx,
             ecx,
             eax;
    uint32_t eip;
    uint16_t cs,
             __pad6;
    uint32_t eflags;
} __attribute__((packed));

#endif // MPX_CONTEXT_H
