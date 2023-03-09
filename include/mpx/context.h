#include <stdint.h>
#ifndef MPX_CONTEXT_H
#define MPX_CONTEXT_H
struct context
{
    uint32_t       gs,
                   fs,
                   es,
                   ds,
                   ss;
    uint32_t      edi,
                  esi,
                  ebp,
                  esp,
                  ebx,
                  edx,
                  ecx,
                  eax;
    uint32_t      eip;

    uint32_t       cs;
    uint32_t   eflags;
} __attribute__((packed));

#endif // MPX_CONTEXT_H
