#ifndef MPX_CONTEXT_H
#define MPX_CONTEXT_H

struct context
{
    unsigned short __pad1,
                   gs,
                   __pad2,
                   fs,
                   __pad3,
                   es,
                   __pad4,
                   ds,
                   __pad5,
                   ss;
    unsigned long edi,
                  esi,
                  ebp,
                  esp,
                  ebx,
                  edx,
                  ecx,
                  eax;
    unsigned long eip;
    unsigned short __pad6,
                   cs;
    unsigned long eflags;
} __attribute__((packed));

#endif // MPX_CONTEXT_H
